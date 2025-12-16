#include "headers/FractalCORE_gateway.h"
#include "headers/FractalCORE_wrapper.h"
#include "headers/Structs&Classes.h"
#include <iostream>
#include <string>

// --- 1. Module-specific structures ---

// Component holding 2D position
struct PositionComponent {
    float x = 0.0f;
    float y = 0.0f;
};

// Event sent when a player/entity moves
struct PlayerMoveEvent {
    Entity entity;
    float newX;
    float newY;
};

// --- 2. System logic ---

// Update function for movement system (high-level C++ logic)
void MovementSystemUpdate(Entity e, PositionComponent& pos, float dt) {
    // Simple movement: advance X based on dt
    pos.x += dt * 10.0f;
    std::cout << "Entity " << e.id << " moved to (" << pos.x << ", " << pos.y << ") | DT: " << dt << std::endl;
}

// Event handler for PlayerMoveEvent
void OnPlayerMove(const PlayerMoveEvent& eventData, void* userData) {
    ModuleAPI* api = static_cast<ModuleAPI*>(userData);

    std::cout << "EVENT RECEIVED: Entity " << eventData.entity.id
              << " moved to (" << eventData.newX << ", " << eventData.newY << ")" << std::endl;

    // Update the Position component via the module API if present
    PositionComponent* pos = api->getComponent<PositionComponent>(eventData.entity, "Position");
    if (pos) {
        pos->x = eventData.newX;
        pos->y = eventData.newY;
    }
}

// --- 3. Module entry points ---

extern "C" {
    // Called by the core when the module is loaded
    void onLoad(FractalCORE_Gateway* gateway) {
        // Use a local static ModuleAPI to ensure it lives until module unload
        static ModuleAPI moduleApi(gateway);

        std::cout << "--- My Module Initialized ---" << std::endl;

        try {
            // Register component type
            moduleApi.registerComponent<PositionComponent>("Position");
            std::cout << "Registered Component: Position" << std::endl;

            // Register an always-running system that iterates over "Position"
            moduleApi.registerSystem<PositionComponent>(
                "Position",            // component name
                MovementSystemUpdate,   // update function
                TriggerType::Always,    // trigger: every frame
                0.0f,                   // timeInterval (unused)
                0                       // tickInterval (unused)
            );
            std::cout << "Registered System: Position_UpdateSystem (Always)" << std::endl;

            // Subscribe to PlayerMove events
            moduleApi.subscribe<PlayerMoveEvent>(
                "PlayerMove",
                OnPlayerMove,
                &moduleApi
            );
            std::cout << "Subscribed to Event: PlayerMove" << std::endl;

            // Create a test entity with a Position component
            Entity player = moduleApi.createEntity();
            moduleApi.attachComponent<PositionComponent>(player, "Position", {10.0f, 5.0f});
            std::cout << "Created test Entity " << player.id << " with Position component." << std::endl;

            // Emit a test PlayerMoveEvent (will immediately call OnPlayerMove)
            PlayerMoveEvent testEvent = {player, 15.0f, 7.0f};
            moduleApi.emitEvent<PlayerMoveEvent>("PlayerMove", testEvent);

        } catch (const std::exception& e) {
            std::cerr << "Module Initialization Error: " << e.what() << std::endl;
        }
    }

    // Called by the core when the module is unloaded (optional)
    void onUnload() {
        std::cout << "--- My Module Unloaded ---" << std::endl;
        // Free any dynamic resources here if needed (EventContexts, SystemModuleContexts)
    }
}