#pragma once
#include "FractalSDK.h"
#include "IKernel.h"
#include "hash/hash.h"
#include <cstdint>

class ECS {
    ECS(std::string name){
        uint32_t domainId = fnv1aHash(name);
        FURCMDPacket packet;
        packet.methodHash = registerCMDomainHash;
        packet.payloadSize = sizeof(uint32_t);
        packet.payload = &domainId;
        FractalSDK::SDK::Get()->sendPacket(packet);
        CMDomainId = domainId;
        CMDomainName = name;
    };
    template<typename T>
    void registerComponent(uint32_t hashId, uint32_t capacity){
        registerComponentCMDContext context;
        context.domainId = CMDomainId;
        context.componentId = hashId;
        context.componentSize = sizeof(T);
        context.capacity = capacity;

        FURCMDPacket packet;
        packet.methodHash = registerComponentHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    };
    template<typename T> 
    void attachComponentDeferred(Entity entity, uint32_t componentHashId, T* componentData){
        attachComponentDeferredCMDContext context;
        context.domainId = CMDomainId;
        context.entity = entity;
        context.componentId = componentHashId;
        context.componentData = componentData;
        context.dataSize = sizeof(T);

        FURCMDPacket packet;
        packet.methodHash = attachComponentDeferredHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;

        FractalSDK::SDK::Get()->sendPacket(packet);
    }
    template<typename T>
    void removeComponentDeferred(Entity entity, uint32_t componentHashId){
        removeComponentDeferredCMDContext context;
        context.domainId = CMDomainId;
        context.entity = entity;
        context.componentId = componentHashId;

        FURCMDPacket packet;
        packet.methodHash = removeComponentDeferredHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;

        FractalSDK::SDK::Get()->sendPacket(packet);
    }
    template<typename T>
    T* getComponent(Entity entity, uint32_t componentHashId){
        getComponentCMDContext context;
        context.domainId = CMDomainId;
        context.entity = entity;
        context.componentId = componentHashId;
        void* output = nullptr;
        
        FURCMDPacket packet;
        packet.methodHash = getComponentHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.outputBuffer = &output;
        FractalSDK::SDK::Get()->sendPacket(packet);
        return static_cast<T*>(output);
    }
    template<typename T>
    Ticket* getComponentAsync(Entity entity, uint32_t componentHashId, void* outputBuffer) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        auto* taskCtx = new GetComponentTask(); 
        taskCtx->context.entity = entity;
        taskCtx->context.componentId = componentHashId;
        taskCtx->context.domainId = CMDomainId;
        taskCtx->ticket = ticket;
        taskCtx->targetBuffer = static_cast<void**>(outputBuffer);

        auto fn = [](void* ctx) {
            auto* data = static_cast<GetComponentTask*>(ctx);
        
            FURCMDPacket packet;
            packet.methodHash = getComponentHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(getComponentCMDContext);
            packet.outputBuffer = data->targetBuffer;
        
            FractalSDK::SDK::Get()->sendPacket(packet);

            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);
        
            delete data;
        };
        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
    }
    template<typename T>
    bool hasComponent(Entity entity, uint32_t componentHashId){
        hasComponentCMDContext context;
        context.domainId = CMDomainId;
        context.entity = entity;
        context.componentId = componentHashId;
        bool exists = false;
        FURCMDPacket packet;
        packet.methodHash = hasComponentHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.outputBuffer = &exists;
        FractalSDK::SDK::Get()->sendPacket(packet);
        return exists;
    }
    template<typename T>
    Ticket* hasComponentAsync(Entity entity, uint32_t componentHashId, void* outputBuffer) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        auto* taskCtx = new hasComponentTask(); 
        taskCtx->context.entity = entity;
        taskCtx->context.componentId = componentHashId;
        taskCtx->context.domainId = CMDomainId;
        taskCtx->ticket = ticket;
        taskCtx->targetBuffer = outputBuffer;

        auto fn = [](void* ctx) {
            auto* data = static_cast<hasComponentTask*>(ctx);
        
            FURCMDPacket packet;
            packet.methodHash = hasComponentHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(hasComponentCMDContext);
            packet.outputBuffer = data->targetBuffer;
        
            FractalSDK::SDK::Get()->sendPacket(packet);

            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);
        
            delete data;
        };
        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
    }
    void registerGroup(std::initializer_list<uint32_t> hashes){
        registerGroupCMDContext context;
        context.componentHashIds = const_cast<uint32_t*>(hashes.begin());
        context.count = hashes.size();
        context.domainId = CMDomainId;
        FURCMDPacket packet;
        packet.methodHash = registerGroupHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(context);
        FractalSDK::SDK::Get()->sendPacket(packet);
    }
    void flushCommands(){
        flushCommandsCMDContext context;
        context.domainId = CMDomainId;
        FURCMDPacket packet;
        packet.payload = &context;
        packet.payloadSize = sizeof(context);
        packet.methodHash = flushCommandsHash;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }
    bool contains(std::initializer_list<uint32_t> array, uint32_t value) {
        containsCMDContext context;
        context.domainId = CMDomainId;
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
    Ticket* containsAsync(std::vector<uint32_t> array, uint32_t value , void* outputBuffer = nullptr){
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        auto* taskCtx = new containsTask();
        taskCtx->hashes = std::move(array);
        taskCtx->context.componentHashIds = taskCtx->hashes.data();
        taskCtx->context.count = taskCtx->hashes.size();
        taskCtx->context.domainId = CMDomainId;
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

            FractalSDK::SDK::Get()->sendPacket(packet);

            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);

            delete data;

        };

        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
    }
    uint32_t getGroupSize(std::initializer_list<uint32_t> hashes) {
        getGroupSizeCMDContext context;
        context.componentHashIds = const_cast<uint32_t*>(hashes.begin());
        context.count = hashes.size();
        context.domainId = CMDomainId;
        uint32_t size = 0;
        FURCMDPacket packet;
        packet.methodHash = getGroupSizeHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(getGroupSizeCMDContext);
        packet.outputBuffer = &size;
        FractalSDK::SDK::Get()->sendPacket(packet);
        return size;
    }
    Ticket* getGroupSizeAsync(std::vector<uint32_t> hashes, void* outputBuffer = nullptr) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        auto* taskCtx = new getGroupSizeTask();

        taskCtx->hashes = std::move(hashes);

        taskCtx->context.componentHashIds = taskCtx->hashes.data();
        taskCtx->context.count = static_cast<uint32_t>(taskCtx->hashes.size());
        taskCtx->context.domainId = CMDomainId;

        taskCtx->targetBuffer = outputBuffer;
        taskCtx->ticket = ticket;

        auto fn = [](void* ctx) {
            auto data = static_cast<getGroupSizeTask*>(ctx);
            FURCMDPacket packet;
            packet.methodHash = getGroupSizeHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(getGroupSizeCMDContext);
            packet.outputBuffer = data->targetBuffer;

            FractalSDK::SDK::Get()->sendPacket(packet);

            if (data->ticket) {
                data->ticket->fence.store(1, std::memory_order_release);
            }
            delete data;
        };

        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx);

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
        FractalSDK::SDK::Get()->sendPacket(packet);
        return rawPtr;
    }
    Ticket* getRawPtrAsync( uint32_t componentId, void* outputBuffer = nullptr) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        auto* taskCtx = new getRawPtrTask();
        taskCtx->ticket = ticket;
        taskCtx->context.componentId = componentId;
        taskCtx->context.domainId = CMDomainId;
        taskCtx->targetBuffer = outputBuffer;
        auto fn = [](void* ctx){
            auto data = static_cast<getRawPtrTask*>(ctx);
            FURCMDPacket packet;
            packet.methodHash = getRawPtrHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(getRawPtrCMDContext);
            packet.outputBuffer = data->targetBuffer;
            FractalSDK::SDK::Get()->sendPacket(packet);
            if (data->ticket) data->ticket->fence.store(1, std::memory_order_release);

            delete data;
        };
        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx);
        return ticket;
    }
    std::string getDomainName(){
        return CMDomainName;
    }
    uint32_t getDomainHashId(){
        return CMDomainId;
    }
private:
    uint32_t CMDomainId;
    std::string CMDomainName;
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
};