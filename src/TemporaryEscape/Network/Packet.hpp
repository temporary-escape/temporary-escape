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

namespace Engine::Network {
class PacketBuffer : public std::vector<unsigned char> {
public:
    void write(const char* src, size_t len) {
        const auto prev = size();
        resize(size() + len);
        std::memcpy(data() + prev, src, len);
    }

    void swap(PacketBuffer& other) noexcept {
        std::vector<unsigned char>::swap(other);
    }
};

inline void swap(PacketBuffer& a, PacketBuffer& b) {
    a.swap(b);
}

struct ENGINE_API Packet {
    uint64_t id{0};
    PacketBuffer data;
};

namespace Details {
// Credits for hash_64_fnv1a_const
// belongs to: https://gist.github.com/underscorediscovery/81308642d0325fd386237cfa3b44785c
constexpr uint64_t val_64_const = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const = 0x100000001b3;

inline constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = val_64_const) noexcept {
    return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
}

template <typename T, typename Container> T unpack(const Container& buffer) {
    msgpack::object_handle obj;
    msgpack::unpack(obj, reinterpret_cast<const char*>(buffer.data()), buffer.size());
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
            try {                                                                                                      \
                return Network::Details::unpack<T>(packet.data);                                                       \
            } catch (...) {                                                                                            \
                debugMessageContents(#T, packet);                                                                      \
                EXCEPTION_NESTED("Failed to unpack message of type '{}'", #T);                                         \
            }                                                                                                          \
        }                                                                                                              \
    };

template <typename T> constexpr uint64_t getMessageId() {
    return Details::MessageTypeId<T>::get();
}

template <typename T> T unpack(const Network::Packet& packet) {
    return Details::MessageUnpacker<T>::unpack(packet);
}

static inline void debugMessageContents(const char* type, const Network::Packet& packet) {
    const auto json = msgpackToJson(packet.data);
    Log::w("Packet", "Packet of type {} failed to unpack, contents: {}", type, json);
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
} // namespace Engine::Network

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Engine::Network::Packet> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Network::Packet& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();
            v.id = o.via.array.ptr[0].as<uint64_t>();
            v.data.resize(o.via.array.ptr[1].via.bin.size);
            std::memcpy(v.data.data(), o.via.array.ptr[1].via.bin.ptr, v.data.size());
            return o;
        }
    };
    template <> struct pack<Engine::Network::Packet> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Network::Packet const& v) const {
            o.pack_array(2);
            o.pack_uint64(v.id);
            o.pack_bin(v.data.size());
            o.pack_bin_body(reinterpret_cast<const char*>(v.data.data()), v.data.size());
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
