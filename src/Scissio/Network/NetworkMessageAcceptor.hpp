#pragma once

#include "../Utils/Exceptions.hpp"
#include "Packet.hpp"

namespace Scissio::Network {
namespace Details {
// Credits for hash_64_fnv1a_const
// belongs to: https://gist.github.com/underscorediscovery/81308642d0325fd386237cfa3b44785c
constexpr uint64_t val_64_const = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const = 0x100000001b3;

inline constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = val_64_const) noexcept {
    return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
}

template <typename T> T unpack(const msgpack::sbuffer& buffer) {
    msgpack::object_handle obj;
    msgpack::unpack(obj, buffer.data(), buffer.size());
    const auto& o = obj.get();

    T ret{};

    try {
        o.convert(ret);
    } catch (...) {
        EXCEPTION_NESTED("Failed to unpack message");
    }
    return ret;
}

template <typename T> T unpack(const Packet& packet) {
    return unpack<T>(packet.data);
}
} // namespace Details

/*class MessageAcceptor {
public:
    virtual ~MessageAcceptor() = default;

    virtual void accept(const SessionPtr& session, const Packet& packet) = 0;
};*/
} // namespace Scissio::Network

#define MESSAGE_REGISTER(T) static constexpr uint64_t KIND = Scissio::Network::Details::hash_64_fnv1a_const(#T);
