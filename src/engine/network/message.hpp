#pragma once

#include "packet.hpp"
#include <typeindex>
#include <unordered_map>

namespace Engine::Network {
template <typename T> struct UseFuture {
    using Type = T;
};

namespace Detail {
ENGINE_API uint64_t getMessageHash(const std::string& name);

template <typename T> struct MessageHelper {};
} // namespace Detail
} // namespace Engine::Network

#define MESSAGE_DEFINE(Type)                                                                                           \
    template <> struct Network::Detail::MessageHelper<Type> {                                                          \
        static inline const uint64_t hash = Network::Detail::getMessageHash(#Type);                                    \
    }
