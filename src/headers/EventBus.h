#pragma once
#include "FractalSDK.h"
#include <string>

class EventBus {
public:
    EventBus(std::string name) {
        domainId = fnv1aHash(name);
        domainName = name;
        
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        FURCMDPacket packet;
        packet.methodHash = registerEBHash;
        packet.payloadSize = sizeof(uint32_t);
        packet.payload = &domainId;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    void subscribe(uint32_t eventId, EventCallback callback, void* user = nullptr, uint32_t subscriberId = 0) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        subscribeEventCMDContext context;
        context.domainId = domainId;
        context.eventId = eventId;
        context.cb = callback;
        context.user = user;
        context.subscriberId = subscriberId;

        FURCMDPacket packet;
        packet.methodHash = subscribeEventHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    void emit(uint32_t eventId, const EventData& data) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        emitEventCMDContext context;
        context.domainId = domainId;
        context.eventId = eventId;
        context.data = data;

        FURCMDPacket packet;
        packet.methodHash = emitEventHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    void push(uint32_t eventId, const EventData& data) {
        pushEventCMDContext context;
        context.domainId = domainId;
        context.eventId = eventId;
        context.data = data;

        FURCMDPacket packet;
        packet.methodHash = pushEventHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;

        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void process() {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        processEventsCMDContext context;
        context.domainId = domainId;

        FURCMDPacket packet;
        packet.methodHash = processEventsHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    void unsubscribe(uint32_t subscriberId) {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        unsubscribeEventCMDContext context;
        context.domainId = domainId;
        context.subscriberId = subscriberId;

        FURCMDPacket packet;
        packet.methodHash = unsubscribeEventHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    void reset() {
        Ticket* ticket = FractalSDK::SDK::Get()->allocateTicket();
        resetEventsCMDContext context;
        context.domainId = domainId;

        FURCMDPacket packet;
        packet.methodHash = resetEventsHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.fence = (uint64_t*)&ticket->fence;

        FractalSDK::SDK::Get()->sendPacket(packet);
        while(!ticket->isReady()) {}
    }

    std::string getDomainName() {
        return domainName;
    }

    uint32_t getDomainHashId() {
        return domainId;
    }

private:
    uint32_t domainId;
    std::string domainName;
};