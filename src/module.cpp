#include "headers/FractalSDK.h"
#include "headers/hash/hash.h"
#include "headers/IKernel.h"
#include "headers/ECS.h"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <iterator>

#ifdef _WIN32
    #define FRACTAL_EXPORT extern "C" __declspec(dllexport)
#else
    #define FRACTAL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

struct exampleComponent {
    float x;
    float y;
    float z;
    float c = 0;
};

FRACTAL_EXPORT void ModuleMain(IKernel* kernel) {
    uint32_t compHash = fnv1aHashConst("exampleComponent");
    FractalSDK::SDK::Initialize(kernel);
    
    ECS cooldomain("new_cool_domain");

    exampleComponent comp = { 1.0f, 5.0f, 20.0f, 0.0f }; 
    cooldomain.registerComponent<exampleComponent>(compHash, 10000);
    
    for (uint32_t i = 0; i < 1000; i++) {
        cooldomain.attachComponentDeferred<exampleComponent>({i}, compHash, &comp);
    }
    
    cooldomain.flushCommands();

    void* data = cooldomain.getRawPtr(compHash);

    exampleComponent* componentdata = (exampleComponent*)data;
    for (uint32_t i = 0; i < 10; i++){
            componentdata[i].x += 0.005f;
        std::cout << componentdata[i].x << std::endl;
    }


    std::cout << "Module logic finished :3" << std::endl;
}