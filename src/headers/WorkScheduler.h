#pragma once
#include "FractalSDK.h"
#include <string>
#include <vector>

class WorkScheduler {
public:
    WorkScheduler(std::string name) {
        domainId = fnv1aHash(name);
        domainName = name;

        FURCMDPacket packet;
        packet.methodHash = registerWSDomainHash;
        packet.payloadSize = sizeof(uint32_t);
        packet.payload = &domainId;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void schedule(w_task_fn fn, void* context) {
        ComputeTask task;
        task.fn = fn;
        task.context = context;

        uint32_t payloadSize = sizeof(task) + sizeof(uint32_t);
        std::vector<uint8_t> buffer(payloadSize);
        
        memcpy(buffer.data(), &domainId, sizeof(uint32_t));
        memcpy(buffer.data() + sizeof(uint32_t), &task, sizeof(task));

        FURCMDPacket packet;
        packet.methodHash = scheduleTaskHash;
        packet.payloadSize = payloadSize;
        packet.payload = buffer.data();
        
        FractalSDK::SDK::Get()->sendPacket(packet);
    }
    std::string getDomainName(){
        return domainName;
    }
    uint32_t getDomainHashId(){
        return domainId;
    }
private:
    uint32_t domainId;
    std::string domainName;
};