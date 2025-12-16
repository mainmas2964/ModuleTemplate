#pragma once
#include "FractalCORE_gateway.h"
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>

// --- ABI Helpers ---

template<typename T>
struct EventContext {
    void (*originalHandler)(const T&, void*);
    void* originalUserData;
    uint32_t eventID;
};

class ModuleAPI {
private:
    FractalCORE_Gateway* gw;
    std::unordered_map<std::string, uint32_t> eventIDCache;

    uint32_t getEventId(const std::string& name) {
        if (eventIDCache.count(name)) return eventIDCache[name];
        uint32_t id = gw->registerEvent(gw->api, name);
        eventIDCache[name] = id;
        return id;
    }

public:
    ModuleAPI(FractalCORE_Gateway* gateway) : gw(gateway) {}

    // ------------------------------------
    // 1. Entity, Task, Time
    // ------------------------------------

    Entity createEntity() {
        return (gw && gw->createEntity) ? gw->createEntity(gw->api) : Entity{0};
    }
    
    void enqueueTask(const Task& task) {
        if (gw && gw->enqueueTask) gw->enqueueTask(gw->api, task);
    }
    
    void registerIntervalTask(const TickTask& tickTask) {
        if (gw && gw->registerIntervalTask) gw->registerIntervalTask(gw->api, tickTask);
    }

    float getDeltaTime() const {
        return (gw && gw->getDeltaTime) ? gw->getDeltaTime(gw->api) : 0.0f;
    }
    
    Clock& getEngineClock() {
        if (!gw || !gw->getEngineClock) throw std::runtime_error("ModuleAPI: Engine Clock unavailable");
        return gw->getEngineClock(gw->api);
    }
    
    void stop() {
        if (gw && gw->stop) gw->stop(gw->api);
    }

    // ------------------------------------
    // 2. Components (ECS)
    // ------------------------------------
     
    template<typename T>
    void registerComponent(const std::string& name, size_t capacity = 10000) {
        if (!gw || !gw->registerComponent) throw std::runtime_error("ModuleAPI: registerComponent null");
        gw->registerComponent(gw->api, name, sizeof(T), capacity);
    }
    
    template<typename T>
    void attachComponent(Entity e, const std::string& name, const T& data) {
        if (!gw || !gw->attachComponent) throw std::runtime_error("ModuleAPI: attachComponent null");
        gw->attachComponent(gw->api, e, name, (void*)&data);
    }

    void removeComponent(Entity e, const std::string& name) {
        if (gw && gw->removeComponent) gw->removeComponent(gw->api, e, name);
    }

    template<typename T>
    T* getComponent(Entity e, const std::string& name) {
        if (!gw || !gw->getComponent) return nullptr;
        return static_cast<T*>(gw->getComponent(gw->api, e, name));
    }
    
    bool hasComponent(Entity e, const std::string& name) {
        return (gw && gw->hasComponent) ? gw->hasComponent(gw->api, e, name) : false;
    }

    // ------------------------------------
    // 3. Systems (ECS)
    // ------------------------------------

    /**
     * @brief Registers a system.
     * Logic: Defines a C-style callback that is called by Core. 
     * Inside that callback, it triggers `updateParallel` to iterate components.
     */
    template<typename T>
    void registerSystem(const std::string& componentName,
                        void (*updateFunc)(Entity, T&, float),
                        TriggerType trigger = TriggerType::Always,
                        float timeInterval = 0.0f, 
                        size_t tickInterval = 0) {
        
        if (!gw || !gw->registerSystem || !gw->registerSystemInLoop || !gw->updateParallel)
            throw std::runtime_error("ModuleAPI: System methods missing");

        // Context to hold function pointer and gateway access
        struct SystemModuleContext {
            std::string compName;
            void (*uFunc)(Entity, T&, float);
            FractalCORE_Gateway* gateway;
        };
        
        // Note: Pointer leaked intentionally here for simplicity (lives as long as app runs)
        auto* moduleContext = new SystemModuleContext{ componentName, updateFunc, gw };
        std::string systemName = componentName + "_UpdateSystem"; 
        
        // Trampoline: Core calls this (C-style), this calls UpdateParallel (std::function)
        auto core_trampoline = [](float dt, void* userData) {
            auto* self = static_cast<SystemModuleContext*>(userData);
            
            // Create lambda for updateParallel
            std::function<void(Entity, void*)> parallel_wrapper = 
                [self, dt](Entity e, void* raw_data) {
                    // Cast raw memory from ComponentData to T&
                    T& component = *static_cast<T*>(raw_data);
                    self->uFunc(e, component, dt);
                };
            
            self->gateway->updateParallel(self->gateway->api, self->compName, parallel_wrapper, 64);
        };

        // 1. Register logic
        gw->registerSystem(gw->api, systemName, core_trampoline, static_cast<void*>(moduleContext));

        // 2. Register execution rules
        SystemDesc desc;
        desc.systemName = systemName;
        desc.trigger = trigger;
        desc.timeInterval = timeInterval;
        desc.tickInterval = tickInterval;
        desc.enabled = true;
        
        gw->registerSystemInLoop(gw->api, desc);
    }
    
    // Direct access to updateParallel
    template<typename T>
    void updateParallel(const std::string& componentName,
                        void (*func)(Entity, T&),
                        size_t chunkSize = 64) {
        if (!gw || !gw->updateParallel) return;
        
        std::function<void(Entity, void*)> wrapper_func = 
            [func](Entity e, void* data) {
                T& component = *static_cast<T*>(data);
                func(e, component);
            };
        
        gw->updateParallel(gw->api, componentName, wrapper_func, chunkSize);
    }

    // ------------------------------------
    // 4. Events
    // ------------------------------------

    template<typename T>
    void subscribe(const std::string& eventName, 
                   void (*handler)(const T&, void*), 
                   void* userData = nullptr) {
        
        if (!gw || !gw->subscribe) return;
            
        uint32_t eventID = getEventId(eventName);
        auto* context = new EventContext<T>{ handler, userData, eventID }; 
        
        auto core_invoker = [](uint32_t id, const EventData& data, void* contextPtr) {
            auto* context = static_cast<EventContext<T>*>(contextPtr);
            if (context && context->originalHandler) {
                // 'ptr' comes from Structs&Classes.h EventData struct
                const T& typedData = *static_cast<const T*>(data.ptr);
                context->originalHandler(typedData, context->originalUserData);
            }
        };

        gw->subscribe(gw->api, eventID, core_invoker, static_cast<void*>(context));
    }
    
    template<typename T>
    void emitEvent(const std::string& eventName, const T& data) {
        if (gw && gw->emitEvent) {
        
            gw->emitEvent(gw->api, getEventId(eventName), (void*)&data, sizeof(T));
        }
    }
    
    template<typename T>
    void pushEvent(const std::string& eventName, const T& data) {
        if (gw && gw->pushEvent) {
        
            gw->pushEvent(gw->api, getEventId(eventName), (void*)&data, sizeof(T));
        }
    }
};