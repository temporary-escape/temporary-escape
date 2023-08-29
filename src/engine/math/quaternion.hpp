#pragma once

#include "../library.hpp"
#include <fmt/format.h>
#include <glm/gtx/quaternion.hpp>
#include <msgpack.hpp>
#include <ostream>
#include <random>

namespace Engine {
using Quaternion = glm::quat;

ENGINE_API Quaternion randomQuaternion(std::mt19937_64& rng);

ENGINE_API void bindMathQuaternion(Lua& lua);
} // namespace Engine

inline std::ostream& operator<<(std::ostream& os, Engine::Quaternion const& v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
    return os;
}

template <> struct fmt::formatter<Engine::Quaternion> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Quaternion const& quat, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}, {}]", quat.x, quat.y, quat.z, quat.w);
    }
};

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Engine::Quaternion> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Quaternion& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(),
                 o.via.array.ptr[1].as<float>(),
                 o.via.array.ptr[2].as<float>(),
                 o.via.array.ptr[3].as<float>()};
            return o;
        }
    };
    template <> struct pack<Engine::Quaternion> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Quaternion const& v) const {
            o.pack_array(4);
            o.pack_float(v.w);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
