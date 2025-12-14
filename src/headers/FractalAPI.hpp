#include "Structs&Classes.h"

#include <functional>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>


#include <algorithm> // Для std::find_if
#include <stdexcept> // Для std::runtime_error

// Forward declarations for types used in member variables
class TaskSystem;
class Clock;
class EntityManager;
class EventBus;
struct Task;
struct TickTask;
struct Module; // Assuming Module is a defined struct/class somewhere

// Forward declarations for template types and Entity alias used in the class
template<typename T>
class Component;

template<typename T>
class SystemECS;

template<typename T>
struct SystemDesc;

using Entity = uint32_t;

// Helper definitions from the implementation block for platform-independent loading
#ifdef _WIN32
    using LibraryHandle = void*; // HMODULE
#else
    using LibraryHandle = void*; // void* from dlopen
#endif

// Structs/Enums assumed to be in "Structs&Classes.h" or included
// For clarity, I'll assume SystemMode and TriggerType are defined in a way
// that makes SystemDesc work.

class FractalAPI {
public:
    // Singleton Access
    static FractalAPI& instance();
    FractalAPI* self();
    
    // Type Aliases
    using CreateModuleFn = Module* (*)(FractalAPI*);

    // Lifecycle Methods
    void run();
    void stop();
    void initialize();
    void shutdown();

    // Initialization Helper Methods (called by initialize)
    void initEntityManager();
    void initEngineClock();
    void initEventBus();
    void initTaskSystem();

    // Task System Methods
    TaskSystem& getTaskSystem();
    void enqueueTask(const Task& task);
    void registerIntervalTask(const TickTask& tickTask);
    void updateIntervalTasks(); // This was in the implementation but not the class, keeping for completeness
    
    // Clock Method
    Clock& getEngineClock();

    // Event System Methods
    template<typename EventType, typename... Args>
    void pushEvent(Args&&... args);
    void processEvents();
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> callback);
    template<typename EventType>
    void emitEvent(const EventType& e);

    // ECS Component Methods
    template<typename T>
    void addECScomponent(size_t maxEntities);
    template<typename T>
    Component<T>* getECScomponent();
    template<typename T>
    void attachECSCompToEntity(Entity e, const T& componentData);

    // ECS System Methods
    template<typename T>
    SystemECS<T>& system();
    template<typename T, typename Func>
    void updateSystemParallel(Func&& func, size_t chunkSize = 64);
    template<typename T, typename Func>
    void updateSystem(Func&& func);
    template<typename T, typename Func>
    void updateSingleCompSystem(Func&& func, Entity e);
    template<typename T, typename Func>
    void registerParallelSystemLoop(Func&& logic, size_t chunkSize = 64); // Deprecated by registerSystem?
    void clearSystems();
    template<typename T>
    void registerSystem(const SystemDesc<T>& desc);
    
    // ECS Entity Methods
    Entity createEntity();
    void getECSentity(uint32_t id = 0);
    
    // Module Manager Methods
    bool loadFromManifest(const std::string& path);
    bool loadModule(const std::string& name);
    bool unloadModule(const std::string& name);

private:
    // Private Constructor/Destructor for Singleton
    FractalAPI();
    ~FractalAPI();
    
    // Prevent copy/move
    FractalAPI(const FractalAPI&) = delete;
    FractalAPI& operator=(const FractalAPI&) = delete;
    FractalAPI(FractalAPI&&) = delete;
    FractalAPI& operator=(FractalAPI&&) = delete;

    // Internal State
    bool initialized = false;
    bool running = false;
    
    // Subsystem pointers (Using unique_ptr requires full definitions in implementation)
    std::unique_ptr<TaskSystem> taskSystem;
    std::unique_ptr<Clock> engineClock;
    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<EventBus> eventBus;
    
    // Initialization Flags
    bool taskSystemInitialized = false;
    bool entityManagerInitialized = false;
    bool engineClockInitialized = false;
    bool eventBusInitialized = false;

    // ECS/System Containers
    std::unordered_map<std::type_index, std::any> systems;
    std::unordered_map<std::type_index, std::any> components;
    std::vector<std::function<void(float)>> systemPipeline;

    // Module Management
    struct LoadedModule {
        std::string name;
        LibraryHandle libraryHandle; // Use type alias
    };
    std::vector<LoadedModule> modules;
};