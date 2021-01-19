#pragma once

#include "../Library.hpp"
#include "../Utils/Msgpack.hpp"

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
    uint64_t sessionId{0};
    msgpack::sbuffer data;

    MSGPACK_DEFINE_ARRAY(id, sessionId, data);
};
} // namespace Scissio::Network
