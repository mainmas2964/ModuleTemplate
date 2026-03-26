#pragma once
#include "FractalSDK.h"
#include <string>

class SQLDB {
public:
    SQLDB(std::string name) {
        domainId = fnv1aHash(name);
        domainName = name;

        registerSQLDomainContext context;
        context.domainId = domainId;
        context.domainName = domainName.c_str();

        FURCMDPacket packet;
        packet.methodHash = registerSQLDomainHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void open(const char* dbPath) {
        openCMDContext context;
        context.domainId = domainId;
        context.dbPath = dbPath;

        FURCMDPacket packet;
        packet.methodHash = openCMDHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void execute(const char* sql) {
        executeCMDContext context;
        context.domainId = domainId;
        context.sql = sql;

        FURCMDPacket packet;
        packet.methodHash = executeCMDHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    void setString(const char* key, const char* value) {
        setStringCMDContext context;
        context.domainId = domainId;
        context.key = key;
        context.value = value;

        FURCMDPacket packet;
        packet.methodHash = setStringCMDHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        FractalSDK::SDK::Get()->sendPacket(packet);
    }

    bool exists(const char* key) {
        existsCMDContext context;
        context.domainId = domainId;
        context.key = key;
        bool result = false;

        FURCMDPacket packet;
        packet.methodHash = existsCMDHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
        packet.outputBuffer = &result;
        FractalSDK::SDK::Get()->sendPacket(packet);
        return result;
    }

    void close() {
        closeCMDContext context;
        context.domainId = domainId;

        FURCMDPacket packet;
        packet.methodHash = closeCMDHash;
        packet.payloadSize = sizeof(context);
        packet.payload = &context;
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