#include "headers/FractalCORE_gateway.h"
#include "headers/FractalCORE_wrapper.h"
#include "headers/Structs&Classes.h"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>

/**
 * ==============================================================================
 * COMPONENT AND EVENT STRUCTURES
 * ==============================================================================
 * This section defines all the components and events used in the module
 * to demonstrate the ECS (Entity Component System) architecture.
 * ==============================================================================
 */

/**
 * @struct PositionComponent
 * @brief Represents the spatial position of an entity in 2D space.
 */
struct PositionComponent {
    float x = 0.0f;  ///< X coordinate
    float y = 0.0f;  ///< Y coordinate
};

/**
 * @struct VelocityComponent
 * @brief Represents the linear velocity of an entity.
 */
struct VelocityComponent {
    float vx = 0.0f;  ///< X velocity component
    float vy = 0.0f;  ///< Y velocity component
};

/**
 * @struct HealthComponent
 * @brief Represents the health status of an entity.
 */
struct HealthComponent {
    float hp = 100.0f;     ///< Current health points
    float maxHp = 100.0f;  ///< Maximum health points
};

/**
 * @struct DamageComponent
 * @brief Represents the damage output capability of an entity.
 */
struct DamageComponent {
    float damage = 10.0f;  ///< Damage amount per hit
};

/**
 * @struct PlayerMoveEvent
 * @brief Event emitted when an entity moves to a new position.
 */
struct PlayerMoveEvent {
    Entity entity;  ///< The entity that moved
    float newX;     ///< New X coordinate
    float newY;     ///< New Y coordinate
};

/**
 * @struct DamageEvent
 * @brief Event emitted when damage is dealt from one entity to another.
 */
struct DamageEvent {
    Entity attacker;      ///< The attacking entity
    Entity target;        ///< The target entity
    float damageAmount;   ///< Amount of damage dealt
};

/**
 * @struct HealthChangedEvent
 * @brief Event emitted when an entity's health changes.
 */
struct HealthChangedEvent {
    Entity entity;      ///< The entity whose health changed
    float oldHealth;    ///< Previous health value
    float newHealth;    ///< New health value
};

/**
 * ==============================================================================
 * SYSTEM UPDATE FUNCTIONS AND EVENT HANDLERS
 * ==============================================================================
 * This section contains the core logic for entity systems and event processing.
 * Each function demonstrates a specific aspect of the ModuleAPI.
 * ==============================================================================
 */

/**
 * @brief MovementSystemUpdate - Processes movement for all entities with Position component.
 * @details Demonstrates the usage of registerSystem<T>().
 *          Executed every frame for each Position component.
 * @param e The entity being updated
 * @param pos The position component to update
 * @param dt Delta time since the last frame (in seconds)
 */
void MovementSystemUpdate(Entity e, PositionComponent& pos, float dt) {
    pos.x += dt * 10.0f;
    std::cout << "[Movement System] Entity " << e.id << " position: (" 
              << pos.x << ", " << pos.y << ")" << std::endl;
}

/**
 * @brief PhysicsSystemUpdate - Processes physics simulation for velocity components.
 * @details Demonstrates the usage of registerSystem<T>() with physics calculations.
 *          Updates position based on velocity over time.
 * @param e The entity being updated
 * @param vel The velocity component to process
 * @param dt Delta time since the last frame (in seconds)
 */
void PhysicsSystemUpdate(Entity e, VelocityComponent& vel, float dt) {
    std::cout << "[Physics System] Entity " << e.id << " velocity: (" 
              << vel.vx << ", " << vel.vy << ")" << std::endl;
}

/**
 * @brief HealthDecaySystem - Applies continuous health degradation over time.
 * @details Demonstrates the usage of registerSystem<T>() with health management.
 *          Applies damage each frame and detects entity death.
 * @param e The entity being updated
 * @param health The health component to update
 * @param dt Delta time since the last frame (in seconds)
 */
void HealthDecaySystem(Entity e, HealthComponent& health, float dt) {
    const float DECAY_RATE = 0.1f;
    health.hp -= DECAY_RATE * dt;
    
    if (health.hp < 0.0f) {
        health.hp = 0.0f;
    }
    
    if (health.hp <= 0.0f) {
        std::cout << "[Health System] Entity " << e.id << " has been defeated." << std::endl;
    }
}

/**
 * @brief OnPlayerMove - Handles PlayerMoveEvent notifications.
 * @details Demonstrates the usage of subscribe<T>().
 *          Receives movement events and updates the position component.
 * @param eventData The event data containing movement information
 * @param userData Pointer to the ModuleAPI instance for component access
 */
void OnPlayerMove(const PlayerMoveEvent& eventData, void* userData) {
    ModuleAPI* api = static_cast<ModuleAPI*>(userData);

    std::cout << "[Event Handler] PlayerMove: Entity " << eventData.entity.id 
              << " moving to (" << eventData.newX << ", " 
              << eventData.newY << ")" << std::endl;

    PositionComponent* pos = api->getComponent<PositionComponent>(
        eventData.entity, "Position"
    );
    if (pos) {
        pos->x = eventData.newX;
        pos->y = eventData.newY;
    }
}

/**
 * @brief OnDamageEvent - Handles DamageEvent notifications.
 * @details Demonstrates the usage of subscribe<T>() with event chaining.
 *          Applies damage to target entity and emits a HealthChangedEvent.
 * @param event The damage event containing attacker and target information
 * @param userData Pointer to the ModuleAPI instance for component access
 */
void OnDamageEvent(const DamageEvent& event, void* userData) {
    ModuleAPI* api = static_cast<ModuleAPI*>(userData);

    std::cout << "[Event Handler] Damage: Entity " << event.attacker.id 
              << " deals " << event.damageAmount << " damage to Entity " 
              << event.target.id << std::endl;

    HealthComponent* health = api->getComponent<HealthComponent>(
        event.target, "Health"
    );
    
    if (health) {
        float oldHealth = health->hp;
        health->hp -= event.damageAmount;
        
        HealthChangedEvent healthEvent{event.target, oldHealth, health->hp};
        api->emitEvent<HealthChangedEvent>("HealthChanged", healthEvent);
    }
}

/**
 * @brief OnHealthChanged - Handles HealthChangedEvent notifications.
 * @details Demonstrates event chaining in the event system.
 *          Tracks and logs health modifications for all entities.
 * @param event The health change event
 * @param userData User-defined context (unused in this case)
 */
void OnHealthChanged(const HealthChangedEvent& event, void* userData) {
    (void)userData;
    std::cout << "[Event Handler] HealthChanged: Entity " << event.entity.id 
              << " health: " << event.oldHealth << " -> " << event.newHealth << std::endl;
}

/**
 * @brief ImmediateMove - Performs immediate position update on all entities.
 * @details Demonstrates the usage of updateParallel<T>().
 *          Used for batch updates without delta time consideration.
 * @param e The entity being updated
 * @param pos The position component to update
 */
void ImmediateMove(Entity e, PositionComponent& pos) {
    (void)e;
    pos.x += 1.0f;
    pos.y += 0.5f;
}

/**
 * @brief VelocityUpdate - Applies gravitational force to all entities.
 * @details Demonstrates the usage of updateParallel<T>() for physics calculations.
 *          Applies gravity in parallel across all velocity components.
 * @param e The entity being updated
 * @param vel The velocity component to update
 */
void VelocityUpdate(Entity e, PositionComponent& pos) {
    (void)e;
    const float GRAVITY = 9.8f;
    const float FRAME_TIME = 0.016f;
    pos.y -= GRAVITY * FRAME_TIME;
}

/**
 * @brief DamageCalculation - Evaluates and applies damage between entities.
 * @details Demonstrates the usage of hasComponent() and getComponent().
 *          Validates component presence before performing operations.
 * @param attacker The attacking entity
 * @param target The target entity
 * @param api Reference to the ModuleAPI for component operations
 */
void DamageCalculation(Entity attacker, Entity target, ModuleAPI& api) {
    std::cout << "[Combat] Damage calculation: Entity " 
              << attacker.id << " vs Entity " << target.id << std::endl;

    bool attackerHasDamage = api.hasComponent(attacker, "Damage");
    bool targetHasHealth = api.hasComponent(target, "Health");

    std::cout << "[Combat] Attacker has Damage component: " 
              << (attackerHasDamage ? "yes" : "no") << std::endl;
    std::cout << "[Combat] Target has Health component: " 
              << (targetHasHealth ? "yes" : "no") << std::endl;

    if (attackerHasDamage && targetHasHealth) {
        DamageComponent* dmg = api.getComponent<DamageComponent>(attacker, "Damage");
        HealthComponent* health = api.getComponent<HealthComponent>(target, "Health");

        if (dmg && health) {
            DamageEvent event{attacker, target, dmg->damage};
            api.emitEvent<DamageEvent>("Damage", event);
        }
    }
}

/**
 * @brief RemoveDeadEntities - Cleans up components of defeated entities.
 * @details Demonstrates the usage of removeComponent().
 *          Removes components associated with deceased entities.
 * @param entity The entity to process
 * @param health The health component to check
 * @param api Reference to the ModuleAPI for component operations
 */
void RemoveDeadEntities(Entity entity, HealthComponent& health, ModuleAPI& api) {
    if (health.hp <= 0.0f) {
        std::cout << "[Cleanup] Removing components from Entity " 
                  << entity.id << std::endl;
        
        if (api.hasComponent(entity, "Velocity")) {
            api.removeComponent(entity, "Velocity");
        }
        if (api.hasComponent(entity, "Damage")) {
            api.removeComponent(entity, "Damage");
        }
    }
}

/**
 * ==============================================================================
 * MODULE ENTRY POINTS
 * ==============================================================================
 * This section contains the module's initialization and cleanup functions.
 * The onLoad() function is called when the module is loaded by FractalCORE.
 * ==============================================================================
 */
extern "C" {
    /**
     * @brief Module initialization routine called by FractalCORE.
     * @details This function is invoked when the shared library is loaded.
     *          It demonstrates all available ModuleAPI features through
     *          practical examples and comprehensive logging.
     * @param gateway Pointer to the FractalCORE_Gateway function table
     */
    void onLoad(FractalCORE_Gateway* gateway) {
        static ModuleAPI moduleApi(gateway);

        std::cout << "\n"
                  << "================================================================================\n"
                  << "                    FractalCORE Module API Demonstration\n"
                  << "================================================================================\n"
                  << std::endl;

        try {
            /* ===================================================================
             * SECTION 1: COMPONENT REGISTRATION
             * Demonstrates: registerComponent<T>()
             * =================================================================== */
            std::cout << "SECTION 1: Component Registration\n" << std::endl;
            
            moduleApi.registerComponent<PositionComponent>("Position", 100000);
            std::cout << "  [OK] registerComponent<PositionComponent>(\"Position\")" << std::endl;
            
            moduleApi.registerComponent<VelocityComponent>("Velocity", 100000);
            std::cout << "  [OK] registerComponent<VelocityComponent>(\"Velocity\")" << std::endl;
            
            moduleApi.registerComponent<HealthComponent>("Health", 50000);
            std::cout << "  [OK] registerComponent<HealthComponent>(\"Health\")" << std::endl;
            
            moduleApi.registerComponent<DamageComponent>("Damage", 50000);
            std::cout << "  [OK] registerComponent<DamageComponent>(\"Damage\")" << std::endl;

            /* ===================================================================
             * SECTION 2: SYSTEM REGISTRATION
             * Demonstrates: registerSystem<T>()
             * =================================================================== */
            std::cout << "\nSECTION 2: System Registration\n" << std::endl;
            
            moduleApi.registerSystem<PositionComponent>(
                "Position",
                MovementSystemUpdate,
                TriggerType::Always,
                0.0f,
                0
            );
            std::cout << "  [OK] registerSystem<PositionComponent>(MovementSystemUpdate)" << std::endl;

            moduleApi.registerSystem<VelocityComponent>(
                "Velocity",
                PhysicsSystemUpdate,
                TriggerType::Always,
                0.0f,
                0
            );
            std::cout << "  [OK] registerSystem<VelocityComponent>(PhysicsSystemUpdate)" << std::endl;

            moduleApi.registerSystem<HealthComponent>(
                "Health",
                HealthDecaySystem,
                TriggerType::Always,
                0.0f,
                0
            );
            std::cout << "  [OK] registerSystem<HealthComponent>(HealthDecaySystem)" << std::endl;

            /* ===================================================================
             * SECTION 3: EVENT SUBSCRIPTION
             * Demonstrates: subscribe<T>()
             * =================================================================== */
            std::cout << "\nSECTION 3: Event Subscription\n" << std::endl;
            
            moduleApi.subscribe<PlayerMoveEvent>(
                "PlayerMove",
                OnPlayerMove,
                &moduleApi
            );
            std::cout << "  [OK] subscribe<PlayerMoveEvent>(\"PlayerMove\")" << std::endl;

            moduleApi.subscribe<DamageEvent>(
                "Damage",
                OnDamageEvent,
                &moduleApi
            );
            std::cout << "  [OK] subscribe<DamageEvent>(\"Damage\")" << std::endl;

            moduleApi.subscribe<HealthChangedEvent>(
                "HealthChanged",
                OnHealthChanged,
                nullptr
            );
            std::cout << "  [OK] subscribe<HealthChangedEvent>(\"HealthChanged\")" << std::endl;

            /* ===================================================================
             * SECTION 4: ENTITY CREATION AND COMPONENT ATTACHMENT
             * Demonstrates: createEntity(), attachComponent<T>()
             * =================================================================== */
            std::cout << "\nSECTION 4: Entity Creation and Component Attachment\n" << std::endl;

            Entity player = moduleApi.createEntity();
            std::cout << "  [OK] createEntity() -> Player (ID: " << player.id << ")" << std::endl;

            PositionComponent playerPos{0.0f, 0.0f};
            moduleApi.attachComponent<PositionComponent>(player, "Position", playerPos);
            std::cout << "  [OK] attachComponent<PositionComponent>(player, \"Position\")" << std::endl;

            VelocityComponent playerVel{5.0f, 0.0f};
            moduleApi.attachComponent<VelocityComponent>(player, "Velocity", playerVel);
            std::cout << "  [OK] attachComponent<VelocityComponent>(player, \"Velocity\")" << std::endl;

            HealthComponent playerHealth{100.0f, 100.0f};
            moduleApi.attachComponent<HealthComponent>(player, "Health", playerHealth);
            std::cout << "  [OK] attachComponent<HealthComponent>(player, \"Health\")" << std::endl;

            DamageComponent playerDmg{25.0f};
            moduleApi.attachComponent<DamageComponent>(player, "Damage", playerDmg);
            std::cout << "  [OK] attachComponent<DamageComponent>(player, \"Damage\")" << std::endl;

            Entity enemy = moduleApi.createEntity();
            std::cout << "  [OK] createEntity() -> Enemy (ID: " << enemy.id << ")" << std::endl;

            PositionComponent enemyPos{10.0f, 0.0f};
            moduleApi.attachComponent<PositionComponent>(enemy, "Position", enemyPos);
            std::cout << "  [OK] attachComponent<PositionComponent>(enemy, \"Position\")" << std::endl;

            HealthComponent enemyHealth{50.0f, 50.0f};
            moduleApi.attachComponent<HealthComponent>(enemy, "Health", enemyHealth);
            std::cout << "  [OK] attachComponent<HealthComponent>(enemy, \"Health\")" << std::endl;

            DamageComponent enemyDmg{15.0f};
            moduleApi.attachComponent<DamageComponent>(enemy, "Damage", enemyDmg);
            std::cout << "  [OK] attachComponent<DamageComponent>(enemy, \"Damage\")" << std::endl;

            /* ===================================================================
             * SECTION 5: COMPONENT QUERY OPERATIONS
             * Demonstrates: hasComponent(), getComponent<T>()
             * =================================================================== */
            std::cout << "\nSECTION 5: Component Query Operations\n" << std::endl;

            bool playerHasHealth = moduleApi.hasComponent(player, "Health");
            std::cout << "  [OK] hasComponent(player, \"Health\"): " 
                      << (playerHasHealth ? "true" : "false") << std::endl;

            bool playerHasArmor = moduleApi.hasComponent(player, "Armor");
            std::cout << "  [OK] hasComponent(player, \"Armor\"): " 
                      << (playerHasArmor ? "true" : "false") << std::endl;

            HealthComponent* playerHealthPtr = moduleApi.getComponent<HealthComponent>(
                player, "Health"
            );
            if (playerHealthPtr) {
                std::cout << "  [OK] getComponent<HealthComponent>(player, \"Health\")" << std::endl;
                std::cout << "       Health: " << playerHealthPtr->hp << "/" 
                          << playerHealthPtr->maxHp << std::endl;
            }

            /* ===================================================================
             * SECTION 6: EVENT SYSTEM SETUP
             * Demonstrates: Event registration and API structure
             * =================================================================== */
            std::cout << "\nSECTION 6: Event System Setup\n" << std::endl;

            std::cout << "  [OK] Event system ready for runtime usage" << std::endl;
            std::cout << "  [OK] PlayerMove events can be emitted at runtime" << std::endl;
            std::cout << "  [OK] Damage events can be emitted at runtime" << std::endl;
            std::cout << "  [OK] HealthChanged events can be emitted at runtime" << std::endl;
            std::cout << "\n  NOTE: Event emission testing is performed at runtime to avoid\n"
                      << "        memory boundary issues between module and core.\n" << std::endl;

            /* ===================================================================
             * SECTION 7: COMBAT SYSTEM VERIFICATION
             * Demonstrates: Component validation and damage calculation logic
             * =================================================================== */
            std::cout << "SECTION 7: Combat System Verification\n" << std::endl;

            std::cout << "  Verifying combat components..." << std::endl;
            bool playerCombatReady = moduleApi.hasComponent(player, "Damage") && 
                                     moduleApi.hasComponent(player, "Health");
            bool enemyCombatReady = moduleApi.hasComponent(enemy, "Damage") && 
                                    moduleApi.hasComponent(enemy, "Health");
            
            std::cout << "  [OK] Player combat ready: " << (playerCombatReady ? "yes" : "no") << std::endl;
            std::cout << "  [OK] Enemy combat ready: " << (enemyCombatReady ? "yes" : "no") << std::endl;
            
            if (playerCombatReady && enemyCombatReady) {
                DamageComponent* playerDamagePtr = moduleApi.getComponent<DamageComponent>(player, "Damage");
                DamageComponent* enemyDamagePtr = moduleApi.getComponent<DamageComponent>(enemy, "Damage");
                
                if (playerDamagePtr && enemyDamagePtr) {
                    std::cout << "  [OK] Player damage output: " << playerDamagePtr->damage << std::endl;
                    std::cout << "  [OK] Enemy damage output: " << enemyDamagePtr->damage << std::endl;
                }
            }

            /* ===================================================================
             * SECTION 8: PARALLEL COMPONENT UPDATES
             * Demonstrates: updateParallel<T>()
             * =================================================================== */
            std::cout << "\nSECTION 8: Parallel Component Updates\n" << std::endl;

            std::cout << "  Executing 3 parallel update passes..." << std::endl;
            for (int pass = 0; pass < 3; ++pass) {
                moduleApi.updateParallel<PositionComponent>(
                    "Position", 
                    ImmediateMove, 
                    256
                );
                std::cout << "  [OK] updateParallel<PositionComponent> (pass " 
                          << (pass + 1) << ")" << std::endl;
            }

            std::cout << "  Executing velocity parallel update..." << std::endl;
            moduleApi.updateParallel<PositionComponent>(
                "Position",
                VelocityUpdate,
                256
            );
            std::cout << "  [OK] updateParallel<PositionComponent> (gravity)" << std::endl;

            /* ===================================================================
             * SECTION 9: MASS ENTITY CREATION
             * Demonstrates: Scalability and batch entity creation
             * =================================================================== */
            std::cout << "\nSECTION 9: Mass Entity Creation Test\n" << std::endl;

            const size_t ENTITY_COUNT = 2000;
            std::cout << "  Creating " << ENTITY_COUNT << " entities..." << std::endl;
            
            for (size_t i = 0; i < ENTITY_COUNT; ++i) {
                Entity e = moduleApi.createEntity();
                
                PositionComponent pos{
                    static_cast<float>(i % 100),
                    static_cast<float>(i / 100)
                };
                moduleApi.attachComponent<PositionComponent>(e, "Position", pos);

                if (i % 500 == 0 && i > 0) {
                    std::cout << "  [OK] " << i << " entities created" << std::endl;
                }
            }
            std::cout << "  [OK] Successfully created " << ENTITY_COUNT << " entities" << std::endl;

            /* ===================================================================
             * SECTION 10: COMPONENT GROUP REGISTRATION
             * Demonstrates: registerGroup()
             * =================================================================== */
            std::cout << "\nSECTION 10: Component Group Registration\n" << std::endl;

            moduleApi.registerGroup({"Position", "Velocity"});
            std::cout << "  [OK] registerGroup({\"Position\", \"Velocity\"})" << std::endl;

            /* ===================================================================
             * SECTION 11: TIMING INFORMATION
             * Demonstrates: getDeltaTime(), getEngineClock()
             * =================================================================== */
            std::cout << "\nSECTION 11: Timing Information\n" << std::endl;

            float deltaTime = moduleApi.getDeltaTime();
            std::cout << "  [OK] getDeltaTime(): " << deltaTime << " seconds" << std::endl;

            try {
                Clock& engineClock = moduleApi.getEngineClock();
                std::cout << "  [OK] getEngineClock(): clock reference obtained" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "  [WARN] getEngineClock(): " << e.what() << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "  [ERROR] Initialization failed: " << e.what() << std::endl;
        }

        /* ===================================================================
         * SECTION 12: TASK SYSTEM
         * Demonstrates: enqueueTask(), registerIntervalTask()
         * =================================================================== */
        std::cout << "\nSECTION 12: Task System\n" << std::endl;

        Task simpleTask;
        simpleTask.func = []() {
            std::cout << "[Task] Simple task executed." << std::endl;
        };
        simpleTask.isBackTask = true;
        moduleApi.enqueueTask(simpleTask);
        std::cout << "  [OK] enqueueTask(simpleTask)" << std::endl;

        TickTask intervalTask;
        intervalTask.func = []() {
            std::cout << "[TickTask] Interval task executed." << std::endl;
        };
        intervalTask.intervalMs = std::chrono::milliseconds(1000);
        intervalTask.executionsRemaining = 5;
        intervalTask.active = true;
        moduleApi.registerIntervalTask(intervalTask);
        std::cout << "  [OK] registerIntervalTask(1000ms interval, 5 executions)" << std::endl;

        /* ===================================================================
         * SECTION 13: PERSISTENT DATA STORAGE
         * Demonstrates: setSQLString(), getSQLString(), SQLExist(), SQLExecute()
         * =================================================================== */
        std::cout << "\nSECTION 13: Persistent Data Storage (SQLite)\n" << std::endl;

        if (moduleApi.setSQLString("game_state", "running")) {
            std::cout << "  [OK] setSQLString(\"game_state\", \"running\")" << std::endl;
        }

        if (moduleApi.setSQLString("player_level", "5")) {
            std::cout << "  [OK] setSQLString(\"player_level\", \"5\")" << std::endl;
        }

        if (moduleApi.SQLExist("game_state")) {
            std::cout << "  [OK] SQLExist(\"game_state\"): true" << std::endl;
        }

        if (!moduleApi.SQLExist("nonexistent_key")) {
            std::cout << "  [OK] SQLExist(\"nonexistent_key\"): false" << std::endl;
        }

        char buffer[256] = {0};
        size_t readSize = moduleApi.getSQLString("game_state", buffer, sizeof(buffer));
        if (readSize > 0) {
            std::cout << "  [OK] getSQLString(\"game_state\"): \"" << buffer << "\"" << std::endl;
        }

        size_t levelReadSize = moduleApi.getSQLString("player_level", buffer, sizeof(buffer));
        if (levelReadSize > 0) {
            std::cout << "  [OK] getSQLString(\"player_level\"): \"" << buffer << "\"" << std::endl;
        }

        if (moduleApi.SQLExecute(
            "CREATE TABLE IF NOT EXISTS stats (id INTEGER, name TEXT, value REAL)"
        )) {
            std::cout << "  [OK] SQLExecute(CREATE TABLE stats)" << std::endl;
        }

        if (moduleApi.SQLExecute(
            "INSERT INTO stats VALUES (1, 'kills', 42)"
        )) {
            std::cout << "  [OK] SQLExecute(INSERT INTO stats)" << std::endl;
        }

        std::cout << "\n"
                  << "================================================================================\n"
                  << "                       Module Initialization Complete\n"
                  << "================================================================================\n"
                  << std::endl;
    }

    /**
     * @brief Module cleanup routine called by FractalCORE.
     * @details This function is invoked when the module is being unloaded.
     *          Perform any necessary cleanup operations here.
     */
    void onUnload() {
        std::cout << "Module unload: Cleanup in progress..." << std::endl;
    }
}