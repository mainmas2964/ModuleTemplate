#pragma once
#include <cstdint>
#include <cstring>
#include <string>
struct IdentityHash {
    using is_avalanching = std::true_type;
    uint32_t operator()(uint32_t x) const noexcept {
        return x;
    }
};

inline uint32_t fnv1aHash(const std::string &str) {
  uint32_t hash = 0x811c9dc5;
  for (char c : str) {
    hash ^= (uint32_t)c;
    hash *= 0x01000193;
  }
  return hash;
}
constexpr uint32_t fnv1aHashConst(const char* str) {
    uint32_t hash = 0x811c9dc5;
    while (*str) {
        hash ^= (uint32_t)(*str++);
        hash *= 0x01000193;
    }
    return hash;
}
