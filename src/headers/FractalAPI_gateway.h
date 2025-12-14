
#pragma once
#include <functional>
#include "Structs&Classes.h"

#ifdef __cplusplus
extern "C" {
#endif

class FractalAPI; 

struct FractalAPI_Gateway {
    FractalAPI* api;
    void (*stop)(FractalAPI*);
    void (*enqueueTask)(FractalAPI*, const Task&);
    Entity (*createEntity)(FractalAPI*);
    float (*getDeltaTime)(FractalAPI*);
};

#ifdef __cplusplus
}
#endif


