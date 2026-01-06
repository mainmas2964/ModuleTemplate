#include "headers/FractalCORE_gateway.h"
#include "headers/FractalCORE_wrapper.h"
#include "headers/Structs&Classes.h"
#include <cstddef>
#include <iostream>
#include <ostream>
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
}

// Event handler for PlayerMoveEvent
void OnPlayerMove(const PlayerMoveEvent& eventData, void* userData) {
    ModuleAPI* api = static_cast<ModuleAPI*>(userData);


    // Update the Position component via the module API if present
    PositionComponent* pos = api->getComponent<PositionComponent>(eventData.entity, "Position");
    if (pos) {
        pos->x = eventData.newX;
        pos->y = eventData.newY;
    }
}

// Immediate update helper used for mass updates (no dt version)
// This will be used with ModuleAPI::updateParallel which expects a
// function pointer of signature void(Entity, T&).
void ImmediateMove(Entity e, PositionComponent& pos) {
    // simple deterministic update to verify updates occurred
    pos.x += 1.0f;
    pos.y += 0.5f;
}

// --- 3. Module entry points ---
extern "C" {
    /**
     * @brief Module entry point called by FractalCORE when the shared library is loaded.
     * @param gateway Pointer to the core's function table.
     */
    void onLoad(FractalCORE_Gateway* gateway) {
        // Static instance ensures the API wrapper persists through the module lifecycle
        static ModuleAPI moduleApi(gateway);

        std::cout << "\n--- My Module Initialized ---" << std::endl;

        try {
            // --- 1. ECS COMPONENT & SYSTEM REGISTRATION ---
            
            // Register a custom 2D position component
            moduleApi.registerComponent<PositionComponent>("Position");
            std::cout << "[ECS] Registered Component: Position" << std::endl;

            // Register a logic system that runs every frame for all "Position" components
            moduleApi.registerSystem<PositionComponent>(
                "Position",            // Target component
                MovementSystemUpdate,   // Function pointer to logic
                TriggerType::Always,    // Execution frequency
                0.0f,                   // Time interval (not used for Always)
                0                       // Tick interval (not used for Always)
            );
            std::cout << "[ECS] Registered System: Position_UpdateSystem (Always)" << std::endl;

            // Subscribe to the "PlayerMove" event
            moduleApi.subscribe<PlayerMoveEvent>(
                "PlayerMove",
                OnPlayerMove,
                &moduleApi
            );
            std::cout << "[ECS] Subscribed to Event: PlayerMove" << std::endl;

            // --- 2. MASS ENTITY CREATION TEST ---
            
            const size_t COUNT = 100000;
            std::cout << "[ECS] Attempting to create " << COUNT << " entities..." << std::endl;
            for (size_t i = 0; i < COUNT; ++i) {
                Entity e = moduleApi.createEntity();
                PositionComponent pc;
                pc.x = static_cast<float>(i);
                pc.y = 0.0f;
                
                moduleApi.attachComponent<PositionComponent>(e, "Position", pc);
                
                // Log progress every 10k entities
                if (((i + 1) % 10000) == 0) {
                    std::cout << "[ECS] Created " << (i + 1) << " entities." << std::endl;
                }
            }

            // --- 3. PARALLEL UPDATE PASSES ---
            
            std::cout << "[ECS] Running 5 immediate parallel update passes..." << std::endl;
            for (int pass = 0; pass < 5; ++pass) {
                // Bulk update Position components using multi-threading
                moduleApi.updateParallel<PositionComponent>("Position", ImmediateMove, 256);
                std::cout << "[ECS] Completed parallel pass " << (pass + 1) << "/5" << std::endl;
            }

        } catch (const std::exception& e) {
            // Catch capacity errors (e.g., if MAX_ENTITIES is exceeded)
            std::cerr << "[ECS] Initialization Error: " << e.what() << std::endl;
        }

        // --- 4. PERSISTENCE & DATABASE INTEGRATION TESTS ---

        std::cout << "\n--- Starting DB Integration Tests ---" << std::endl;

        // A. Redis Test (In-Memory Cache)
        const char* redisKey = "test_key";
        const char* redisVal = "new_redis_value";
        
        moduleApi.setRedisString(redisKey, redisVal);
        
        char redisBuff[256] = {0};
        if (moduleApi.getRedisString(redisKey, redisBuff, sizeof(redisBuff)) > 0) {
            std::cout << "[REDIS] Read successful: " << redisBuff << std::endl;
        }

        // B. SQLite Test (Persistent Storage)
        const char* sqlKey = "test_db_key";
        const char* sqlVal = "122";
        
        // Try to read existing data first
        char sqlBuff[256] = {0};
        size_t sqlReadSize = moduleApi.getSQLString(sqlKey, sqlBuff, sizeof(sqlBuff));
        
        if (sqlReadSize > 0) {
            std::cout << "[SQLITE] Found existing data: " << sqlBuff << std::endl;
        } else {
            std::cout << "[SQLITE] No data found. Writing fresh value..." << std::endl;
        }

        // Perform a write operation to the disk
        if (moduleApi.setSQLString(sqlKey, sqlVal)) {
            std::cout << "[SQLITE] Successfully wrote value: " << sqlVal << std::endl;
        }

        // C. Sync Test (Redis to SQLite data transfer)
        char syncTemp[256] = {0};
        if (moduleApi.getRedisString(redisKey, syncTemp, sizeof(syncTemp)) > 0) {
            moduleApi.setSQLString("sync_backup", syncTemp);
            std::cout << "[SYNC] Backed up Redis value '" << syncTemp << "' to SQLite." << std::endl;
        }

        std::cout << "--- All Tests Completed ---\n" << std::endl;
    }

    /**
     * @brief Cleanup function called when the module is being unloaded.
     */
    void onUnload() {
        std::cout << "--- My Module Unloaded ---" << std::endl;
    }
}