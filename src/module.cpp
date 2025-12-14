#include "headers/FractalAPI_gateway.h"
#include <iostream>

static FractalAPI_Gateway* g_gateway = nullptr;

extern "C" {

void onLoad(FractalAPI_Gateway* gateway) {
    g_gateway = gateway;  
    std::cout << "Module loaded, API pointer via gateway: " << g_gateway->api << std::endl;

    g_gateway->enqueueTask(g_gateway->api, Task{
        .func = [](){ std::cout << "Task from module executed!\n"; }
    });

    Entity e = g_gateway->createEntity(g_gateway->api);
    std::cout << "Created entity ID: " << e.id << std::endl;
}

} // extern "C"

