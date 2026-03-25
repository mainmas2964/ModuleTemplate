#pragma once
#include <cstdint>

struct FURCMDPacket;
typedef void (*FURMethod)(FURCMDPacket& packet);

class IKernel {
public:
    virtual ~IKernel() = default;
    
    virtual void sendCMDPacket(FURCMDPacket& packet) = 0;
    virtual void registerCMDMethod(uint32_t hashId, FURMethod method) = 0;
};