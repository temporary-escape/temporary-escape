#pragma once

#include "../Library.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Msgpack.hpp"
#include <functional>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<msgpack::sbuffer> {
        msgpack::object const& operator()(msgpack::object const& o, msgpack::sbuffer& v) const {
            if (o.type != msgpack::type::BIN)
                throw msgpack::type_error();
            v.write(o.via.bin.ptr, o.via.bin.size);
            return o;
        }
    };

    template <> struct pack<msgpack::sbuffer> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, msgpack::sbuffer const& v) const {
            o.pack_bin(static_cast<uint32_t>(v.size()));
            o.pack_bin_body(v.data(), static_cast<uint32_t>(v.size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack

namespace Scissio::Network {
struct SCISSIO_API Packet {
    uint64_t id{0};
    msgpack::sbuffer data;

    MSGPACK_DEFINE_ARRAY(id, data);
};

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

template <typename T> struct MessageTypeId {
    static constexpr uint64_t get() {
        return 0ULL;
    }
};

template <typename T> struct MessageUnpacker {
    static T unpack(const Network::Packet& packet) {
        (void)packet;
        EXCEPTION("You must register message: '{}' before unpacking it!", typeid(T).name());
    }
};
} // namespace Details

#define REGISTER_MESSAGE(T)                                                                                            \
    template <> struct Network::Details::MessageTypeId<T> {                                                            \
        static constexpr uint64_t get() {                                                                              \
            return hash_64_fnv1a_const(#T);                                                                            \
        }                                                                                                              \
    };                                                                                                                 \
    template <> struct Network::Details::MessageUnpacker<T> {                                                          \
        static T unpack(const Network::Packet& packet) {                                                               \
            return Network::Details::unpack<T>(packet.data);                                                           \
        }                                                                                                              \
    };

template <typename T> constexpr uint64_t getMessageId() {
    return Details::MessageTypeId<T>::get();
}

template <typename T> T unpack(const Network::Packet& packet) {
    return Details::MessageUnpacker<M>::unpack(packet);
}

template <typename... Args> class MessageDispatcher {
public:
    using Callback = std::function<void(Args&&..., const Network::Packet&)>;

    template <typename M> void add(std::function<void(Args..., M)> fn) {
        auto callback = [fn{std::move(fn)}](Args&&... args, const Network::Packet& packet) {
            fn(std::forward<Args>(args)..., unpack<M>(packet));
        };

        callbacks.insert(std::make_pair(getMessageId<M>(), std::move(callback)));
    };

    void dispatch(Args&&... args, const Network::Packet& packet) {
        const auto it = callbacks.find(packet.id);
        if (it == callbacks.end()) {
            EXCEPTION("Failed to handle packet id: {} no callback defined", packet.id);
        }
        it->second(std::forward<Args>(args)..., packet);
    }

private:
    std::unordered_map<uint64_t, Callback> callbacks;
};
} // namespace Scissio::Network
