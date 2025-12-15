
#pragma once
#include <functional>
#include "Structs&Classes.h"

struct FractalAPI_Gateway {
    void* api; 
    

    void (*stop)(void*);
    void (*enqueueTask)(void*, const Task&);
    Entity (*createEntity)(void*);
    float (*getDeltaTime)(void*);
    

    bool (*registerComponentType)(void* api, 
                                  const char* typeName,
                                  size_t typeSize,
                                  size_t typeAlign);
    

    bool (*createComponent)(void* api, 
                           const char* typeName, 
                           size_t maxEntities);
    

    bool (*attachComponent)(void* api,
                           const char* typeName,
                           Entity entity,
                           const void* data);
    

    void* (*getComponent)(void* api,
                         const char* typeName,
                         Entity entity);
    

    bool (*registerSystemByName)(void* api,
                                 const char* componentType,
                                 void (*updateFunc)(Entity, void*, float, void*),
                                 void* userData,
                                 int mode, // 0=seq, 1=parallel
                                 int trigger, // 0=always, 1=time, 2=tick
                                 float timeInterval,
                                 size_t tickInterval);
    

    bool (*registerEventType)(void* api, const char* eventName, size_t eventSize);
    bool (*subscribeEvent)(void* api, const char* eventName, 
                          void (*handler)(const void*, void*), void* userData);
    void (*emitEvent)(void* api, const char* eventName, const void* data);
};
