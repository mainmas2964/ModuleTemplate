// В модуле
#include "headers/FractalAPI_gateway.h"
#include "headers/FractalAPI_wrapper.h"
#include "headers/Structs&Classes.h"
#include <iostream>

static FractalAPI_Gateway* g_gateway = nullptr;

struct Transform {
    float x = 0;
    float y = 0;
    float z = 0;
};

void taskFunction() {
    std::cout << "Task from module executed!\n";
}

void updateTransform(Entity e, Transform& t, float dt) {
    // Простая физика - движение вправо
    t.x += 1.0f * dt;
    
    // Каждую секунду выводим первую сущность
    static float timer = 0;
    static bool printed = false;
    timer += dt;
    if (timer >= 1.0f && !printed) {
        if (e.id == 1) {
            std::cout << "[System] Entity " << e.id << " at (" 
                      << t.x << ", " << t.y << ", " << t.z << ")" << std::endl;
        }
        printed = true;
    }
    if (timer >= 1.0f && e.id == 1) {
        timer = 0;
        printed = false;
    }
}

extern "C" {
void onLoad(FractalAPI_Gateway* gateway) {
    g_gateway = gateway;  
    std::cout << "=== MODULE LOADING ===" << std::endl;
    
    ModuleAPI api(g_gateway);
    

    std::cout << "\n=== REGISTERING COMPONENT ===" << std::endl;
    api.registerComponent<Transform>("Transform", 100000);
    

    std::cout << "\n=== CREATING ENTITIES ===" << std::endl;
    const size_t NUM_ENTITIES = 100000;
    
    for (size_t i = 0; i < NUM_ENTITIES; ++i) {
        Entity e = g_gateway->createEntity(g_gateway->api);
        Transform t{
            static_cast<float>(i * 10), 
            static_cast<float>(i * 20), 
            static_cast<float>(i * 30)
        };
        api.addComponent("Transform", e, t);
    }
    
    std::cout << "Created " << NUM_ENTITIES << " entities" << std::endl;
    

    std::cout << "\n=== REGISTERING PARALLEL SYSTEM ===" << std::endl;
    api.registerSystem<Transform>("Transform", updateTransform, true); // true = parallel
    
    std::cout << "\n=== MODULE LOADED SUCCESSFULLY ===" << std::endl;
}
}