#ifndef STRUCTUSES_AND_CLASSES_HEADER_
#define STRUCTUSES_AND_CLASSES_HEADER_
#include <stdint.h>
#include <functional>
#include <chrono>
#include <string>
struct Entity
{
    uint32_t id;
};
struct Task{
    std::function<void()> func;
    bool isBackTask = true;
};
struct TickTask{
    std::function<void()> func;
    std::chrono::milliseconds intervalMs;
    std::chrono::steady_clock::time_point lastExecutedMs;
    size_t id;
    size_t executionsRemaining;
    bool active;
    float timeAccumulator = 0.0f;
    bool isBackTask=true;

};
class Event {
    public:
    virtual ~Event() = default;
};
template<typename T>
class SystemECS;

enum class SystemMode {
    Serial,
    Parallel
};

enum class TriggerType {
    Always,
    TimeInterval,
    TickInterval
};

template<typename T>
struct SystemDesc {
    std::function<void(Entity, T&)> callback;
    std::string name = "Unnamed System";
    SystemMode mode = SystemMode::Serial;
    TriggerType trigger = TriggerType::Always;
    float timeInterval = 0.0f;
    size_t tickInterval = 0;
    size_t chunkSize = 64;
};

#pragma once
class Module {
public:
    virtual ~Module() = default;
    virtual void onLoad() {}
    virtual void onUnload() {}
};
#endif
