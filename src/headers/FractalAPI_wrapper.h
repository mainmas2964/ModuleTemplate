#pragma once
#include "FractalAPI_gateway.h"
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <string>

class ModuleAPI {
    FractalAPI_Gateway* gw;
    

    struct SystemCallback {
        void* funcPtr;
        void (*invoker)(void*, Entity, void*, float);
    };
    
    static std::unordered_map<std::string, SystemCallback>& getSystemRegistry() {
        static std::unordered_map<std::string, SystemCallback> registry;
        return registry;
    }
    
public:
    ModuleAPI(FractalAPI_Gateway* gateway) : gw(gateway) {}
    
    template<typename T>
    bool registerComponent(const char* name, size_t maxEntities = 10000) {
        std::cout << "[ModuleAPI] Registering component: " << name << "\n";
        
        if (!gw || !gw->registerComponentType || !gw->createComponent) {
            std::cout << "[ModuleAPI] ERROR: Gateway or functions are null!\n";
            return false;
        }
        
        if (!gw->registerComponentType(gw->api, name, sizeof(T), alignof(T))) {
            std::cout << "[ModuleAPI] Failed to register component type\n";
            return false;
        }
        
        bool result = gw->createComponent(gw->api, name, maxEntities);
        std::cout << "[ModuleAPI] createComponent returned: " << result << "\n";
        return result;
    }
    
    template<typename T>
    bool addComponent(const char* name, Entity e, const T& data) {
        if (!gw || !gw->attachComponent) return false;
        return gw->attachComponent(gw->api, name, e, &data);
    }
    
    template<typename T>
    T* getComponent(const char* name, Entity e) {
        if (!gw || !gw->getComponent) return nullptr;
        return static_cast<T*>(gw->getComponent(gw->api, name, e));
    }
    
    Entity createEntity() {
        if (!gw || !gw->createEntity) return Entity{0};
        return gw->createEntity(gw->api);
    }
    
    template<typename T>
    bool registerSystem(const char* componentName,
                       void (*updateFunc)(Entity, T&, float),
                       bool parallel = false) {
        
        if (!gw || !gw->registerSystemByName) return false;
        
        std::string key(componentName);
        

        auto invoker = +[](void* funcPtr, Entity e, void* data, float dt) {
            auto actualFunc = reinterpret_cast<void(*)(Entity, T&, float)>(funcPtr);
            T* component = static_cast<T*>(data);
            actualFunc(e, *component, dt);
        };
        

        getSystemRegistry()[key] = SystemCallback{
            reinterpret_cast<void*>(updateFunc),
            invoker
        };
        

        static auto wrapper = +[](Entity e, void* data, float dt, void* userData) {
            const char* compName = static_cast<const char*>(userData);
            if (!compName) return;
            
            auto& registry = getSystemRegistry();
            auto it = registry.find(std::string(compName));
            if (it != registry.end()) {
                it->second.invoker(it->second.funcPtr, e, data, dt);
            }
        };
        

        static std::string persistentName = componentName;
        
        return gw->registerSystemByName(gw->api, componentName, wrapper, 
                                        (void*)persistentName.c_str(),
                                        parallel ? 1 : 0, 0, 0.0f, 0);
    }
    
    void stop() {
        if (gw && gw->stop) {
            gw->stop(gw->api);
        }
    }
};