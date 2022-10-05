#pragma once

#include "../Library.hpp"
#include "../Utils/Msgpack.hpp"
#include "NetworkAsio.hpp"

namespace Engine {
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
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Engine::Packet> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Packet& v) const {
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
    template <> struct pack<Engine::Packet> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Packet const& v) const {
            o.pack_array(2);
            o.pack_uint64(v.id);
            o.pack_bin(static_cast<uint32_t>(v.data.size()));
            o.pack_bin_body(reinterpret_cast<const char*>(v.data.data()), v.data.size());
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
