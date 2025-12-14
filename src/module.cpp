// module.cpp
// Example module for Fractal engine. This file exports an extern "C"
// onLoad(Fractal::FractalAPI*) entry-point. The engine will call onLoad when
// the module is loaded. The module must interact only with the provided
// FractalAPI pointer and keep all its internal data private.

#include "headers/FractalAPI.hpp"
#include <iostream>

FractalAPI* api = nullptr;

extern "C" {

// Called by the engine when the module is loaded. The module should only
// use the provided api pointer. It must not assume anything about engine
// internals.
void onLoad(FractalAPI* fapi) {
    api = fapi;
    std::cout << "ExampleModule loaded" << std::endl;
}

} // extern "C"

