#pragma once

#include "FractalCORE_gateway.h"
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>
#include <vector>

/**
 * @brief Context holder for ABI-stable event dispatching.
 */
template<typename T>
struct EventContext {
    void (*originalHandler)(const T&, void*);
    void* originalUserData;
    uint32_t eventID;
};

/**
 * @brief High-level C++ Wrapper for the FractalCORE Gateway API.
 * Provides a type-safe interface for ECS, Task Scheduling, and Event systems.
 */
class ModuleAPI {
private:
    FractalCORE_Gateway* m_gw;
    std::unordered_map<std::string, uint32_t> m_eventIDCache;

    uint32_t getEventId(const std::string& name) {
        auto it = m_eventIDCache.find(name);
        if (it != m_eventIDCache.end()) return it->second;
        
        uint32_t id = m_gw->registerEvent(m_gw->api, name);
        m_eventIDCache[name] = id;
        return id;
    }

public:
    explicit ModuleAPI(FractalCORE_Gateway* gateway) : m_gw(gateway) {
        if (!m_gw) throw std::invalid_argument("ModuleAPI: Gateway pointer cannot be null");
    }

    // --- Core Management ---

    Entity createEntity() {
        return (m_gw && m_gw->createEntity) ? m_gw->createEntity(m_gw->api) : Entity{0};
    }
    
    void enqueueTask(const Task& task) {
        if (m_gw && m_gw->enqueueTask) m_gw->enqueueTask(m_gw->api, task);
    }
    
    void registerIntervalTask(const TickTask& tickTask) {
        if (m_gw && m_gw->registerIntervalTask) m_gw->registerIntervalTask(m_gw->api, tickTask);
    }

    float getDeltaTime() const {
        return (m_gw && m_gw->getDeltaTime) ? m_gw->getDeltaTime(m_gw->api) : 0.0f;
    }
    
    Clock& getEngineClock() {
        if (!m_gw || !m_gw->getEngineClock) throw std::runtime_error("ModuleAPI: Engine Clock unavailable");
        return m_gw->getEngineClock(m_gw->api);
    }
    
    void stop() {
        if (m_gw && m_gw->stop) m_gw->stop(m_gw->api);
    }

    // --- ECS: Component Management ---
     
    template<typename T>
    void registerComponent(const std::string& name, size_t capacity = 10000) {
        if (!m_gw || !m_gw->registerComponent) throw std::runtime_error("ModuleAPI: registerComponent unavailable");
        m_gw->registerComponent(m_gw->api, name, sizeof(T), capacity);
    }
    
    template<typename T>
    void attachComponent(Entity e, const std::string& name, const T& data) {
        if (!m_gw || !m_gw->attachComponent) throw std::runtime_error("ModuleAPI: attachComponent unavailable");
        m_gw->attachComponent(m_gw->api, e, name, (void*)&data);
    }

    void removeComponent(Entity e, const std::string& name) {
        if (m_gw && m_gw->removeComponent) m_gw->removeComponent(m_gw->api, e, name);
    }

    template<typename T>
    T* getComponent(Entity e, const std::string& name) {
        if (!m_gw || !m_gw->getComponent) return nullptr;
        return static_cast<T*>(m_gw->getComponent(m_gw->api, e, name));
    }
    
    bool hasComponent(Entity e, const std::string& name) {
        return (m_gw && m_gw->hasComponent) ? m_gw->hasComponent(m_gw->api, e, name) : false;
    }

    // --- ECS: System & Parallel Processing ---

    /**
     * @brief Processes multiple components in parallel groups.
     */
    void updateParallelGroup(const std::vector<std::string>& componentNames,
                             void (*updateFunc)(size_t start, size_t end, void* userCtx),
                             void* userCtx = nullptr,
                             size_t chunkSize = 1024) {
        if (m_gw && m_gw->updateParallelGroup) {
            m_gw->updateParallelGroup(m_gw->api, componentNames, updateFunc, userCtx, chunkSize);
        }
    }

    void registerGroup(const std::vector<std::string>& componentNames) {
        if (m_gw && m_gw->registerGroup) {
            m_gw->registerGroup(m_gw->api, componentNames);
        }
    }

    /**
     * @brief High-level system registration with automatic trampoline creation.
     */
    template<typename T>
    void registerSystem(const std::string& componentName,
                        void (*updateFunc)(Entity, T&, float),
                        TriggerType trigger = TriggerType::Always,
                        float timeInterval = 0.0f, 
                        size_t tickInterval = 0) {
        
        struct SystemModuleContext {
            std::string compName;
            void (*uFunc)(Entity, T&, float);
            FractalCORE_Gateway* gateway;
            float currentDt;
        };
        
        auto* moduleContext = new SystemModuleContext{ componentName, updateFunc, m_gw, 0.0f };
        std::string systemName = componentName + "_UpdateSystem"; 

        // Trampoline to bridge C-style callback with typed component data
        auto core_trampoline = [](float dt, void* userData) {
            auto* ctx = static_cast<SystemModuleContext*>(userData);
            ctx->currentDt = dt; 
            
            auto entity_callback = [](Entity e, void* raw_data, void* userCtx) {
                auto* sCtx = static_cast<SystemModuleContext*>(userCtx);
                T& component = *static_cast<T*>(raw_data);
                sCtx->uFunc(e, component, sCtx->currentDt);
            };

            ctx->gateway->updateParallel(ctx->gateway->api, ctx->compName, entity_callback, ctx, 64);
        };

        m_gw->registerSystem(m_gw->api, systemName, core_trampoline, static_cast<void*>(moduleContext));

        SystemDesc desc;
        desc.systemName = systemName;
        desc.trigger = trigger;
        desc.timeInterval = timeInterval;
        desc.tickInterval = tickInterval;
        desc.enabled = true;
        
        m_gw->registerSystemInLoop(m_gw->api, desc);
    }
    
    /**
     * @brief Direct parallel iteration over a specific component type.
     */
    template<typename T>
    void updateParallel(const std::string& componentName,
                        void (*func)(Entity, T&),
                        size_t chunkSize = 64) {
        if (!m_gw || !m_gw->updateParallel) return;

        struct SimpleFuncCtx {
            void (*f)(Entity, T&);
        };
        auto* ctx = new SimpleFuncCtx{ func };

        auto wrapper_func = [](Entity e, void* data, void* userCtx) {
            auto* sCtx = static_cast<SimpleFuncCtx*>(userCtx);
            T& component = *static_cast<T*>(data);
            sCtx->f(e, component);
        };
        
        m_gw->updateParallel(m_gw->api, componentName, wrapper_func, ctx, chunkSize);
    }

    // --- Messaging & Events ---

    /**
     * @brief Subscribes to a global event with type safety.
     */
    template<typename T>
    void subscribe(const std::string& eventName, 
                   void (*handler)(const T&, void*), 
                   void* userData = nullptr) {
        
        if (!m_gw || !m_gw->subscribe) return;
            
        uint32_t eventID = getEventId(eventName);
        auto* context = new EventContext<T>{ handler, userData, eventID }; 
        
        auto core_invoker = [](uint32_t id, const EventData& data, void* contextPtr) {
            auto* context = static_cast<EventContext<T>*>(contextPtr);
            if (context && context->originalHandler) {
                const T& typedData = *static_cast<const T*>(data.ptr);
                context->originalHandler(typedData, context->originalUserData);
            }
        };

        m_gw->subscribe(m_gw->api, eventID, core_invoker, static_cast<void*>(context));
    }
    
    template<typename T>
    void emitEvent(const std::string& eventName, const T& data) {
        if (m_gw && m_gw->emitEvent) {
            m_gw->emitEvent(m_gw->api, getEventId(eventName), (void*)&data, sizeof(T));
        }
    }
    
    template<typename T>
    void pushEvent(const std::string& eventName, const T& data) {
        if (m_gw && m_gw->pushEvent) {
            m_gw->pushEvent(m_gw->api, getEventId(eventName), (void*)&data, sizeof(T));
        }
    }
};