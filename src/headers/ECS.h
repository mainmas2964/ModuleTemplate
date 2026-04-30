#pragma once
#include "FractalSDK.h"
#include "IKernel.h"
#include "hash/hash.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

class ECS {
public:
    ECS(std::string name) {
        CMDomainId = fnv1aHash(name);
        CMDomainName = name;

        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        FURCMDPacket packet;
        packet.methodHash = registerCMDomainHash;
        packet.payloadSize = sizeof(uint32_t);
        packet.payload = &CMDomainId;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    template<typename T>
    void registerComponent(uint32_t hashId, uint32_t capacity) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        registerComponentCMDContext context;
        context.domainId = CMDomainId;
        context.componentId = hashId;
        context.componentSize = sizeof(T);
        context.capacity = capacity;

        FURCMDPacket packet;
        packet.methodHash = registerComponentHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    template<typename T> 
    void attachComponentDeferred(Entity entity, uint32_t componentHashId, T* componentData) {
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
    void removeComponentDeferred(Entity entity, uint32_t componentHashId) {
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

    void flushCommands() {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        flushCommandsCMDContext context;
        context.domainId = CMDomainId;

        FURCMDPacket packet;
        packet.methodHash = flushCommandsHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(context);
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    template<typename T>
    T* getComponent(Entity entity, uint32_t componentHashId) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
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
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
        return static_cast<T*>(output);
    }

    bool hasComponent(Entity entity, uint32_t componentHashId) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
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
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
        return exists;
    }

    void* getRawPtr(uint32_t componentId, uint32_t domainId = 0) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        getRawPtrCMDContext context;
        context.componentId = componentId;
        context.domainId = (domainId == 0) ? CMDomainId : domainId;
        
        void* rawPtr = nullptr;
        FURCMDPacket packet;
        packet.methodHash = getRawPtrHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(getRawPtrCMDContext);
        packet.outputBuffer = &rawPtr;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
        return rawPtr;
    }

    uint32_t getGroupSize(std::initializer_list<uint32_t> hashes) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        getGroupSizeCMDContext context;
        context.componentHashIds = const_cast<uint32_t*>(hashes.begin());
        context.count = (uint32_t)hashes.size();
        context.domainId = CMDomainId;

        uint32_t size = 0;
        FURCMDPacket packet;
        packet.methodHash = getGroupSizeHash;
        packet.payload = &context;
        packet.payloadSize = sizeof(getGroupSizeCMDContext);
        packet.outputBuffer = &size;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
        return size;
    }

    bool contains(std::initializer_list<uint32_t> array, uint32_t value) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        containsCMDContext context;
        context.domainId = CMDomainId;
        context.componentHashIds = const_cast<uint32_t*>(array.begin());
        context.value = value;
        context.count = (uint32_t)array.size();
        
        bool result = false;
        FURCMDPacket packet;
        packet.methodHash = containsHash;
        packet.outputBuffer = &result;
        packet.payload = &context;
        packet.payloadSize = sizeof(context);
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
        return result;
    }

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
            packet.fence = (uint64_t*)&data->ticket->fence;
            FractalSDK::SDK::Get()->sendPacket(packet);
            delete data;
        };
        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
    }

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
            packet.fence = (uint64_t*)&data->ticket->fence;
            FractalSDK::SDK::Get()->sendPacket(packet);
            delete data;
        };
        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx, 0);
        return ticket;
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
            packet.fence = (uint64_t*)&data->ticket->fence;
            FractalSDK::SDK::Get()->sendPacket(packet);
            delete data;
        };
        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx);
        return ticket; 
    }

    Ticket* getRawPtrAsync(uint32_t componentId, void* outputBuffer = nullptr) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        auto* taskCtx = new getRawPtrTask();
        taskCtx->ticket = ticket;
        taskCtx->context.componentId = componentId;
        taskCtx->context.domainId = CMDomainId;
        taskCtx->targetBuffer = outputBuffer;

        auto fn = [](void* ctx) {
            auto data = static_cast<getRawPtrTask*>(ctx);
            FURCMDPacket packet;
            packet.methodHash = getRawPtrHash;
            packet.payload = &data->context;
            packet.payloadSize = sizeof(getRawPtrCMDContext);
            packet.outputBuffer = data->targetBuffer;
            packet.fence = (uint64_t*)&data->ticket->fence;
            FractalSDK::SDK::Get()->sendPacket(packet);
            delete data;
        };
        FractalSDK::WorkScheduler::scheduleTask(fn, taskCtx);
        return ticket;
    }

    std::string getDomainName() { return CMDomainName; }
    uint32_t getDomainHashId() { return CMDomainId; }

private:
    uint32_t CMDomainId;
    std::string CMDomainName;

    struct GetComponentTask { getComponentCMDContext context; Ticket* ticket; void** targetBuffer; };
    struct hasComponentTask { hasComponentCMDContext context; Ticket* ticket; void* targetBuffer; };
    struct containsTask { containsCMDContext context; Ticket* ticket; void* targetBuffer; std::vector<uint32_t> hashes; };
    struct getGroupSizeTask { getGroupSizeCMDContext context; Ticket* ticket; void* targetBuffer; std::vector<uint32_t> hashes; };
    struct getRawPtrTask { getRawPtrCMDContext context; Ticket* ticket; void* targetBuffer; };
};