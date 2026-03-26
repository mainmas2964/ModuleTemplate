#pragma once
#include "FractalSDK.h"
#include <string>

class EventBus {
public:
    EventBus(std::string name) {
        domainId = fnv1aHash(name);
        domainName = name;
        
        FURCMDPacket packet;
        packet.methodHash = registerEBHash;
        packet.payloadSize = sizeof(uint32_t);
        packet.payload = &domainId;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void subscribe(uint32_t eventId, EventCallback callback, void* user = nullptr, uint32_t subscriberId = 0) {
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
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void emit(uint32_t eventId, const EventData& data) {
        emitEventCMDContext context;
        context.domainId = domainId;
        context.eventId = eventId;
        context.data = data;

        FURCMDPacket packet;
        packet.methodHash = emitEventHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
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
        processEventsCMDContext context;
        context.domainId = domainId;

        FURCMDPacket packet;
        packet.methodHash = processEventsHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void unsubscribe(uint32_t subscriberId) {
        unsubscribeEventCMDContext context;
        context.domainId = domainId;
        context.subscriberId = subscriberId;

        FURCMDPacket packet;
        packet.methodHash = unsubscribeEventHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void reset() {
        resetEventsCMDContext context;
        context.domainId = domainId;

        FURCMDPacket packet;
        packet.methodHash = resetEventsHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

private:
    uint32_t domainId;
    std::string domainName;
};