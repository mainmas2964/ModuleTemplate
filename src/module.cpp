#include "headers/FractalSDK.h"
#include "headers/hash/hash.h"
#include "headers/IKernel.h" // Обязательно подключаем интерфейс
#include <iostream>
#include <iterator>

// Экспортируем функции
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

// ТЕПЕРЬ: Принимаем IKernel* вместо KernelAPI
FRACTAL_EXPORT void ModuleMain(IKernel* kernel) {

    FractalSDK::SDK::Initialize(kernel);
    
    uint32_t compHash = fnv1aHashConst("exampleComponent");
    exampleComponent comp= exampleComponent{ 1.0f, 5.0f, 20.0f}; 
    
    FractalSDK::ECS::registerComponent<exampleComponent>(compHash, 10);
    FractalSDK::ECS::attachComponentDeferred(Entity{1}, fnv1aHashConst("exampleComponent") , &comp );
    std::cout << "done." << std::endl;
    FractalSDK::ECS::flushCommands(0);
    std::cout << "flush done." << std::endl;

    exampleComponent* getComp = FractalSDK::ECS::getComponent<exampleComponent>(Entity{1}, compHash);
    if (FractalSDK::ECS::hasComponent<exampleComponent>( {1}, fnv1aHashConst("exampleComponent"))){
        std::cout << "Has" << std::endl;
    };
    
    if (getComp == nullptr) {
        std::cout << "Error: getComponent returned nullptr!" << std::endl;
    } else {
        // Если здесь падает Bus error - значит адрес в getComp не кратен 4 (размер float)
        std::cout << getComp->x << " " << getComp->y << " " << getComp->z << std::endl; 
    }


    std::cout << "Hello, World! (hello from module via IKernel :3)" << std::endl;
}