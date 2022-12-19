#pragma once

#include "packet.hpp"
#include <typeindex>
#include <unordered_map>

namespace Engine::Network::Detail {
ENGINE_API uint64_t getMessageHash(const std::string& name);
} // namespace Engine::Network::Detail

#define MESSAGE_DEFINE(Type, ...)                                                                                      \
    static inline const uint64_t hash = Network::Detail::getMessageHash(#Type);                                        \
    MSGPACK_DEFINE_ARRAY(__VA_ARGS__);
