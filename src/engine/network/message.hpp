#pragma once

#include "packet.hpp"
#include <typeindex>
#include <unordered_map>

namespace Engine::Network {
template <typename T> struct UseFuture {
    using Type = T;
};

namespace Detail {
ENGINE_API uint64_t getMessageHash(const std::string_view& name);

template <typename T> struct MessageHelper {};
} // namespace Detail

class ENGINE_API RawMessage {
public:
    explicit RawMessage(std::shared_ptr<msgpack::object_handle> oh) : oh{std::move(oh)} {
    }

    [[nodiscard]] const msgpack::object& get() const {
        if (!oh) {
            throw std::bad_cast();
        }
        return oh->get().via.array.ptr[1];
    }

private:
    std::shared_ptr<msgpack::object_handle> oh;
};
} // namespace Engine::Network

#define MESSAGE_DEFINE(Type)                                                                                           \
    template <> struct Network::Detail::MessageHelper<Type> {                                                          \
        static inline const uint64_t hash = Network::Detail::getMessageHash(#Type);                                    \
    }
