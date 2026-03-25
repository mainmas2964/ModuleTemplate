#include "hash/hash.h"
#include <cstdint>
#include <initializer_list>
#include <utility>
#include <vector>
#include <atomic>
#include <memory>
#include <iostream>
#include "IKernel.h"
// --- FUR CMD PACKET - FAST UTILITY ROUTER COMMAND PACKET ---

struct FURCMDPacket {
    uint32_t methodHash;    // ID of the method being invoked
    uint16_t payloadSize;   // Size of input data (up to 64 KB)
    uint16_t flags;         // System flags (e.g., priority or call type)

    void* payload;          // Pointer to input data (arguments)

    void* outputBuffer;     // Pointer to buffer where the engine will write the response

    uint64_t* fence = nullptr;      // Pointer to an atomic synchronization ticket
};
typedef void (*FURMethod) (FURCMDPacket& packet);


// --- ECS Contexts ---
//

struct Entity {
    uint32_t id;
};

struct attachComponentDeferredCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
    void* componentData;
    uint32_t dataSize;
};

struct removeComponentDeferredCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct registerComponentCMDContext {
    uint32_t domainId;
    uint32_t componentId;
    uint32_t componentSize;
    uint32_t capacity;
};

struct flushCommandsCMDContext {
    uint32_t domainId;
};

struct getComponentCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct hasComponentCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct registerGroupCMDContext {
    uint32_t domainId;
    uint32_t* componentHashIds;
    uint32_t count;
};

struct containsCMDContext {
    uint32_t domainId;
    uint32_t* componentHashIds;
    uint32_t count;
    uint32_t value;
};

struct hasAllCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t* componentHashIds;
    uint32_t count;
};

struct getGroupSizeCMDContext {
    uint32_t domainId;
    uint32_t* componentHashIds;
    uint32_t count;
};

struct getRawPtrCMDContext {
    uint32_t domainId;
    uint32_t componentId;
};

struct onComponentAttachedCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct onComponentRemovedCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};


// -- Work Scheduler Contexts ---
//
typedef void (*w_task_fn)(void* context);
struct ComputeTask {
    w_task_fn fn;
    void* context;
};

// --- SQL Database Contexts ---
//

struct registerSQLDomainContext {
    uint32_t domainId;

    const char* domainName;
};
struct openCMDContext {

    uint32_t domainId;

    const char* dbPath;

};
struct executeCMDContext {
    uint32_t domainId;

    const char* sql;
};
struct setStringCMDContext {
    uint32_t domainId;

    const char* key;

    const char* value;
};
struct getStringCMDContext {
    uint32_t domainId;

    const char* key;

    uint32_t bufferSize;

};
struct existsCMDContext {
    uint32_t domainId;

    const char* key;

};
struct closeCMDContext {
    uint32_t domainId;

};
struct isOpenCMDContext {
    uint32_t domainId;

};


// --- Event System Contexts ---
//

struct EventData {
    void* ptr;
    size_t size;
}; //Event Data Context


typedef void (*EventCallback)(uint32_t, const EventData&, void*);

struct Subscriber {
    EventCallback cb;
    void* user;
    uint32_t subscriberId;
};

struct QueuedEvent {
    uint32_t id;
    EventData data;
};

struct subscribeEventCMDContext {
    uint32_t domainId;
    uint32_t eventId;
    EventCallback cb;
    void* user;
    uint32_t subscriberId;
};
struct emitEventCMDContext {
    uint32_t domainId;
    uint32_t eventId;
    EventData data;
};
struct pushEventCMDContext {
    uint32_t domainId;
    uint32_t eventId;
    EventData data;
};
struct processEventsCMDContext {
    uint32_t domainId;
};
struct resetEventsCMDContext {
    uint32_t domainId;
};
struct unsubscribeEventCMDContext {
    uint32_t domainId;
    uint32_t subscriberId;
};
// --- Module Loader Contexts ---
// 

struct loadModuleContext {
    const char* path;
};
struct isLoadedCMDContext {
    const char* path;
};

struct KernelAPI {
    void (*sendCMDPacket)(void* packetPtr);
    void (*registerCMDMethod)(uint32_t hashId, void* methodPtr);
};
typedef void (*ModuleEntry)(KernelAPI api);


// METHOD HASHES

// --- ECS ---
constexpr uint32_t registerCMDomainHash = fnv1aHashConst("fractal_engine:ecs:registerCMDomain");
constexpr uint32_t flushCommandsHash = fnv1aHashConst("fractal_engine:ecs:flushCommands");
constexpr uint32_t registerComponentHash = fnv1aHashConst("fractal_engine:ecs:registerComponent");
constexpr uint32_t attachComponentDeferredHash = fnv1aHashConst("fractal_engine:ecs:attachComponentDeferred");
constexpr uint32_t removeComponentDeferredHash = fnv1aHashConst("fractal_engine:ecs:removeComponentDeferred");
constexpr uint32_t getComponentHash = fnv1aHashConst("fractal_engine:ecs:getComponent");
constexpr uint32_t hasComponentHash = fnv1aHashConst("fractal_engine:ecs:hasComponent");
constexpr uint32_t onComponentAttachedHash = fnv1aHashConst("fractal_engine:ecs:onComponentAttached");
constexpr uint32_t onComponentRemovedHash = fnv1aHashConst("fractal_engine:ecs:onComponentRemoved");
constexpr uint32_t registerGroupHash = fnv1aHashConst("fractal_engine:ecs:registerGroup");
constexpr uint32_t containsHash = fnv1aHashConst("fractal_engine:ecs:contains");
constexpr uint32_t hasAllHash = fnv1aHashConst("fractal_engine:ecs:hasAll");
constexpr uint32_t getGroupSizeHash = fnv1aHashConst("fractal_engine:ecs:getGroupSize");
constexpr uint32_t getRawPtrHash = fnv1aHashConst("fractal_engine:ecs:getRawPtr");

// --- Work Scheduler ---

constexpr uint32_t registerWSDomainHash = fnv1aHashConst("fractal_engine:work_scheduler:registerWSDomain");
constexpr uint32_t scheduleTaskHash = fnv1aHashConst("fractal_engine:work_scheduler:scheduleTask");

// --- SQL Database ---

constexpr uint32_t registerSQLDomainHash = fnv1aHashConst("fractal_engine:sqldb:registerSQLDomain");
constexpr uint32_t openCMDHash = fnv1aHashConst("fractal_engine:sqldb:openCMD");
constexpr uint32_t executeCMDHash = fnv1aHashConst("fractal_engine:sqldb:executeCMD");
constexpr uint32_t setStringCMDHash = fnv1aHashConst("fractal_engine:sqldb:setStringCMD");
constexpr uint32_t getStringCMDHash = fnv1aHashConst("fractal_engine:sqldb:getStringCMD");
constexpr uint32_t existsCMDHash = fnv1aHashConst("fractal_engine:sqldb:existsCMD");
constexpr uint32_t closeCMDHash = fnv1aHashConst("fractal_engine:sqldb:closeCMD");
constexpr uint32_t isOpenCMDHash = fnv1aHashConst("fractal_engine:sqldb:isOpenCMD");

// --- Event System ---

constexpr uint32_t registerEBHash = fnv1aHashConst("fractal_engine:event_bus:registerEBDomain");
constexpr uint32_t subscribeEventHash = fnv1aHashConst("fractal_engine:event_bus:subscribeEvent");
constexpr uint32_t emitEventHash = fnv1aHashConst("fractal_engine:event_bus:emitEvent");
constexpr uint32_t pushEventHash = fnv1aHashConst("fractal_engine:event_bus:pushEvent");
constexpr uint32_t processEventsHash = fnv1aHashConst("fractal_engine:event_bus:processEvents");
constexpr uint32_t resetEventsHash = fnv1aHashConst("fractal_engine:event_bus:resetEvents");
constexpr uint32_t unsubscribeEventHash = fnv1aHashConst("fractal_engine:event_bus:unsubscribeEvent");

// --- Module Loader ---

constexpr uint32_t loadModuleHash = fnv1aHashConst("fractal_engine:module_loader:loadModule");
constexpr uint32_t isLoadedHash = fnv1aHashConst("fractal_engine:module_loader:isLoaded");

// --- Fractal SDK Header ---
// Ticket Pool

struct Ticket {
    std::atomic<uint64_t> fence{0};
    uint32_t id = 0;

    void reset() { fence.store(0, std::memory_order_release); }
    bool isReady() const { return fence.load(std::memory_order_acquire) == 1; }
};

class TicketPool {
public:
    TicketPool(size_t size = 1024) : m_tickets(size) {
        for(uint32_t i = 0; i < size; ++i) m_tickets[i].id = i;
    }

    Ticket* acquire() {
        uint32_t index = m_next.fetch_add(1) % m_tickets.size();
        Ticket* t = &m_tickets[index];
        t->reset(); 
        return t;
    }

private:
    std::vector<Ticket> m_tickets;
    std::atomic<uint32_t> m_next{0};
};

// FractalSDK
namespace FractalSDK {
namespace WorkScheduler{
    void scheduleTask(w_task_fn fn, void* context, uint32_t domainId = 0);
    void registerDomain(uint32_t domainId);
};
class SDK {
public:
    SDK(const SDK&) = delete;
    SDK& operator=(const SDK&) = delete;

    static void Initialize(IKernel* kernel) {
        if (!instance) {
            instance = new SDK(kernel);
        }
    }

    static void Shutdown() {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    Ticket* allocateTicket() {
        return m_ticketPool->acquire();
    }

    static SDK* Get() { return instance; }

    void sendPacket(FURCMDPacket& packet) {
        if (!instance) {
            std::cerr << "[SDK Error] Instance is NULL! Call Initialize first." << std::endl;
            return; 
        }
        if (!m_kernel) {
            std::cerr << "[SDK Error] IKernel pointer is NULL!" << std::endl;
            return;
        }

        m_kernel->sendCMDPacket(packet);
    }

    void registerMethod(uint32_t hash, FURMethod method) {
        if (!m_kernel) {
            std::cerr << "[SDK Error] IKernel pointer is NULL!" << std::endl;
            return;
        }
        m_kernel->registerCMDMethod(hash, method);
    }

private:
    explicit SDK(IKernel* kernel) : m_kernel(kernel) {
        m_ticketPool = std::make_unique<TicketPool>(1024);
    }
    
    ~SDK() = default;

    std::unique_ptr<TicketPool> m_ticketPool;
    
    IKernel* m_kernel; 
    
    static inline SDK* instance = nullptr;
};


namespace ECS {
    struct GetComponentTask {
        getComponentCMDContext context;
        Ticket* ticket;
        void** targetBuffer;
    };
    struct hasComponentTask {
        hasComponentCMDContext context;
        Ticket* ticket;
        void* targetBuffer;
    };
    struct containsTask {
        containsCMDContext context;
        Ticket* ticket;
        void* targetBuffer;
        std::vector<uint32_t> hashes;
    };
    struct getGroupSizeTask {
        getGroupSizeCMDContext context;
        Ticket* ticket;
        void* targetBuffer;
        std::vector<uint32_t> hashes;
    };
    struct getRawPtrTask {
        getRawPtrCMDContext context;
        Ticket* ticket;
        void* targetBuffer;
    };
    template<typename T>
    void registerComponent(uint32_t hashId, uint32_t capacity, uint32_t domainId = 0){
        registerComponentCMDContext context;
        context.domainId = domainId;
        context.componentId = hashId;
        context.componentSize = sizeof(T);
        context.capacity = capacity;

        FURCMDPacket packet;
        packet.methodHash = registerComponentHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        SDK::Get()->sendPacket(packet);
    }
    template<typename T> 
    void attachComponentDeferred(Entity entity, uint32_t componentHashId, T* componentData, uint32_t domainId = 0){
        attachComponentDeferredCMDContext context;
        context.domainId = domainId;
        context.entity = entity;
        context.componentId = componentHashId;
        context.componentData = componentData;
        context.dataSize = sizeof(T);

        FURCMDPacket packet;
        packet.methodHash = attachComponentDeferredHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;

        SDK::Get()->sendPacket(packet);
    }
    template<typename T>
    void removeComponentDeferred(Entity entity, uint32_t componentHashId, uint32_t domainId = 0){
        removeComponentDeferredCMDContext context;
        context.domainId = domainId;
        context.entity = entity;
        context.componentId = componentHashId;

        FURCMDPacket packet;
        packet.methodHash = removeComponentDeferredHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;

        SDK::Get()->sendPacket(packet);
    }
    template<typename T>
    T* getComponent(Entity entity, uint32_t componentHashId, uint32_t domainId = 0){
        getComponentCMDContext context;
        context.domainId = domainId;
        context.entity = entity;
        context.componentId = componentHashId;
        void* output = nullptr;
        
        FURCMDPacket packet;
        packet.methodHash = getComponentHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.outputBuffer = &output;
        SDK::Get()->sendPacket(packet);
        return static_cast<T*>(output);
    }
    template<typename T>
    Ticket* getComponentAsync(Entity entity, uint32_t componentHashId, void* outputBuffer, uint32_t domainId = 0) {
        Ticket* ticket = SDK::Get()->allocateTicket();
        auto* taskCtx = new GetComponentTask(); 
        taskCtx->context.entity = entity;
        taskCtx->context.componentId = componentHashId;
        taskCtx->context.domainId = domainId;
        taskCtx->ticket = ticket;
        taskCtx->targetBuffer = static_cast<void**>(outputBuffer);

        auto fn = [](void* ctx) {
            auto* data = static_cast<GetComponentTask*>(ctx);
        
            FURCMDPacket packet;
            packet.methodHash = getComponentHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(getComponentCMDContext);
            packet.outputBuffer = data->targetBuffer;
        
            SDK::Get()->sendPacket(packet);

            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);
        
            delete data;
        };
        WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
    }
    template<typename T>
    bool hasComponent(Entity entity, uint32_t componentHashId, uint32_t domainId = 0){
        hasComponentCMDContext context;
        context.domainId = domainId;
        context.entity = entity;
        context.componentId = componentHashId;
        bool exists = false;
        FURCMDPacket packet;
        packet.methodHash = hasComponentHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.outputBuffer = &exists;
        SDK::Get()->sendPacket(packet);
        return exists;
    }
    template<typename T>
    Ticket* hasComponentAsync(Entity entity, uint32_t componentHashId, void* outputBuffer, uint32_t domainId = 0) {
        Ticket* ticket = SDK::Get()->allocateTicket();
        auto* taskCtx = new hasComponentTask(); 
        taskCtx->context.entity = entity;
        taskCtx->context.componentId = componentHashId;
        taskCtx->context.domainId = domainId;
        taskCtx->ticket = ticket;
        taskCtx->targetBuffer = outputBuffer;

        auto fn = [](void* ctx) {
            auto* data = static_cast<hasComponentTask*>(ctx);
        
            FURCMDPacket packet;
            packet.methodHash = hasComponentHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(hasComponentCMDContext);
            packet.outputBuffer = data->targetBuffer;
        
            SDK::Get()->sendPacket(packet);

            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);
        
            delete data;
        };
        WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
    }
    void registerGroup(std::initializer_list<uint32_t> hashes, uint32_t domainId = 0){
        registerGroupCMDContext context;
        context.componentHashIds = const_cast<uint32_t*>(hashes.begin());
        context.count = hashes.size();
        context.domainId = domainId;
        FURCMDPacket packet;
        packet.methodHash = registerGroupHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(context);
        SDK::Get()->sendPacket(packet);
    }
    void flushCommands(uint32_t domainId = 0){
        flushCommandsCMDContext context;
        context.domainId = domainId;
        FURCMDPacket packet;
        packet.payload = &context;
        packet.payloadSize = sizeof(context);
        packet.methodHash = flushCommandsHash;
        SDK::Get()->sendPacket(packet);
    }
    bool contains(std::initializer_list<uint32_t> array, uint32_t value,uint32_t domainId = 0) {
        containsCMDContext context;
        context.domainId = domainId;
        context.componentHashIds = const_cast<uint32_t*>(array.begin());
        context.value = value;
        context.count = array.size();
        bool contains = false;
        FURCMDPacket packet;
        packet.methodHash = containsHash;
        packet.outputBuffer = &contains;
        packet.payload = &context;
        packet.payloadSize = sizeof(context);
        return contains;
    }
    Ticket* containsAsync(std::vector<uint32_t> array, uint32_t value , void* outputBuffer = nullptr, uint32_t domainId = 0){
        Ticket* ticket = SDK::Get()->allocateTicket();
        auto* taskCtx = new containsTask();
        taskCtx->hashes = std::move(array);
        taskCtx->context.componentHashIds = taskCtx->hashes.data();
        taskCtx->context.count = taskCtx->hashes.size();
        taskCtx->context.domainId = domainId;
        taskCtx->context.value = value;
        taskCtx->targetBuffer = outputBuffer;
        taskCtx->ticket = ticket;

        auto fn = [](void* ctx){
            auto data = static_cast<containsTask*>(ctx);
            FURCMDPacket packet;
            packet.methodHash = containsHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(containsCMDContext);
            packet.outputBuffer = data->targetBuffer;

            SDK::Get()->sendPacket(packet);

            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);

            delete data;

        };

        WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
    }
    uint32_t getGroupSize(std::initializer_list<uint32_t> hashes, uint32_t domainId) {
        getGroupSizeCMDContext context;
        context.componentHashIds = const_cast<uint32_t*>(hashes.begin());
        context.count = hashes.size();
        context.domainId = domainId;
        uint32_t size = 0;
        FURCMDPacket packet;
        packet.methodHash = getGroupSizeHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(getGroupSizeCMDContext);
        packet.outputBuffer = &size;
        SDK::Get()->sendPacket(packet);
        return size;
    }
    Ticket* getGroupSizeAsync(std::vector<uint32_t> hashes, void* outputBuffer = nullptr, uint32_t domainId = 0) {
        Ticket* ticket = SDK::Get()->allocateTicket();
        auto* taskCtx = new getGroupSizeTask();

        taskCtx->hashes = std::move(hashes);

        taskCtx->context.componentHashIds = taskCtx->hashes.data();
        taskCtx->context.count = static_cast<uint32_t>(taskCtx->hashes.size());
        taskCtx->context.domainId = domainId;

        taskCtx->targetBuffer = outputBuffer;
        taskCtx->ticket = ticket;

        auto fn = [](void* ctx) {
            auto data = static_cast<getGroupSizeTask*>(ctx);
            FURCMDPacket packet;
            packet.methodHash = getGroupSizeHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(getGroupSizeCMDContext);
            packet.outputBuffer = data->targetBuffer;

            SDK::Get()->sendPacket(packet);

            if (data->ticket) {
                data->ticket->fence.store(1, std::memory_order_release);
            }
            delete data;
        };

        WorkScheduler::scheduleTask(fn, taskCtx);

        return ticket; 
    }
    void* getRawPtr(uint32_t componentId, uint32_t domainId = 0) {
        getRawPtrCMDContext context;
        context.componentId = componentId;
        context.domainId = domainId;
        void* rawPtr = nullptr;
        FURCMDPacket packet;
        packet.methodHash = getRawPtrHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(getRawPtrCMDContext);
        packet.outputBuffer = &rawPtr;
        SDK::Get()->sendPacket(packet);
        return rawPtr;
    }
    Ticket* getRawPtrAsync( uint32_t componentId, void* outputBuffer = nullptr,uint32_t domainId = 0) {
        Ticket* ticket = SDK::Get()->allocateTicket();
        auto* taskCtx = new getRawPtrTask();
        taskCtx->ticket = ticket;
        taskCtx->context.componentId = componentId;
        taskCtx->context.domainId = domainId;
        taskCtx->targetBuffer = outputBuffer;
        auto fn = [](void* ctx){
            auto data = static_cast<getRawPtrTask*>(ctx);
            FURCMDPacket packet;
            packet.methodHash = getRawPtrHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(getRawPtrCMDContext);
            packet.outputBuffer = data->targetBuffer;
            SDK::Get()->sendPacket(packet);
            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);

            delete data;
        };
        WorkScheduler::scheduleTask(fn, taskCtx);
        return ticket;
    }

}

namespace Event {
    void registerEventBus(uint32_t eventBusId){
        FURCMDPacket packet;
        packet.methodHash = registerEBHash;
        packet.payload = &eventBusId;
        packet.payloadSize = sizeof(uint32_t);
        SDK::Get()->sendPacket(packet);
    }
    void subscribe(uint32_t eventId, EventCallback callback, void* user = nullptr, uint32_t subscriberId = 0, uint32_t domainId = 0){
        subscribeEventCMDContext context;
        context.eventId = eventId;
        context.subscriberId = subscriberId;
        context.cb = callback;
        context.domainId = domainId;
        context.user = user;
        FURCMDPacket packet;
        packet.methodHash = subscribeEventHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(subscribeEventCMDContext);
        SDK::Get()->sendPacket(packet);
    }
    void emitEvent(uint32_t eventId, const EventData& data, uint32_t domainId = 0){
        emitEventCMDContext context;
        context.data = data;
        context.eventId = eventId;
        context.domainId = domainId;
        FURCMDPacket packet;
        packet.methodHash = emitEventHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(emitEventCMDContext);
        SDK::Get()->sendPacket(packet);
    }
    void pushEvent(uint32_t eventId, const EventData& data, uint32_t domainId = 0) {
        pushEventCMDContext context;
        context.data = data;
        context.eventId = eventId;
        context.domainId = domainId;
        FURCMDPacket packet;
        packet.methodHash = pushEventHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(pushEventCMDContext);
        SDK::Get()->sendPacket(packet);
    }
    void processEvents(uint32_t domainId) {
        processEventsCMDContext context;
        context.domainId = domainId;
        FURCMDPacket packet;
        packet.methodHash = processEventsHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(processEventsCMDContext);
        SDK::Get()->sendPacket(packet);
    }
    void unsubscribe(uint32_t subscriberId, uint32_t domainId) {
        unsubscribeEventCMDContext context;
        context.domainId = domainId;
        context.subscriberId = subscriberId;
        FURCMDPacket packet;
        packet.methodHash = unsubscribeEventHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(unsubscribeEventCMDContext);
        SDK::Get()->sendPacket(packet);
    }
    void reset(uint32_t domainId) {
        resetEventsCMDContext context;
        context.domainId = domainId;
        FURCMDPacket packet;
        packet.methodHash = resetEventsHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(resetEventsCMDContext);
    }
}
namespace SQLDB {
    void registerSQLDomain(){}
}
namespace WorkScheduler {
    void scheduleTask(w_task_fn fn, void* context, uint32_t domainId){
        ComputeTask task;
        task.fn = fn;
        task.context = context;

        FURCMDPacket packet;
        packet.methodHash = scheduleTaskHash;
        packet.payloadSize = sizeof(task) + sizeof(domainId);
        std::vector<uint8_t> payload(packet.payloadSize);
        memcpy(payload.data(), &domainId, sizeof(domainId));
        memcpy(payload.data() + sizeof(domainId), &task, sizeof(task));
        packet.payload = payload.data();

        SDK::Get()->sendPacket(packet);
    }
    void registerDomain(uint32_t domainId){
        FURCMDPacket packet;
        packet.methodHash = registerWSDomainHash;
        packet.payloadSize = sizeof(domainId);
        packet.payload = &domainId;

        SDK::Get()->sendPacket(packet);
    }


//
}
//
}