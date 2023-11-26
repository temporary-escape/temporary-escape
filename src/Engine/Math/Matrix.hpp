#pragma once

#include "Quaternion.hpp"
#include "Vector.hpp"
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <msgpack.hpp>

namespace Engine {
using Matrix4 = glm::mat4x4;
using Matrix3 = glm::mat3x3;

ENGINE_API Matrix4 withoutScale(const Matrix4& mat);

ENGINE_API void bindMathMatrices(Lua& lua);
} // namespace Engine

template <> struct fmt::formatter<Engine::Matrix3> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Matrix3 const& mat, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}]", mat[0], mat[1], mat[2]);
    }
};

template <> struct fmt::formatter<Engine::Matrix4> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Matrix4 const& mat, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}, {}]", mat[0], mat[1], mat[2], mat[3]);
    }
};

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Engine::Matrix4> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Matrix4& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 16)
                throw msgpack::type_error();
            // v = { o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>(),
            //     o.via.array.ptr[3].as<float>() };
            auto dst = const_cast<float*>(&v[0].x);
            for (auto i = 0; i < 16; i++) {
                dst[i] = o.via.array.ptr[i].as<float>();
            }
            return o;
        }
    };
    template <> struct pack<Engine::Matrix4> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Matrix4 const& v) const {
            o.pack_array(16);
            const auto src = &v[0].x;
            for (auto i = 0; i < 16; i++) {
                o.pack_float(src[i]);
            }
            return o;
            ;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
