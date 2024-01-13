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
using Matrix4 = glm::f32mat4x4;
using Matrix4d = glm::f64mat4x4;
using Matrix3 = glm::f32mat3x3;
using Matrix3d = glm::f64mat3x3;

ENGINE_API Matrix4 withoutScale(const Matrix4& mat);
} // namespace Engine

template <> struct fmt::formatter<Engine::Matrix3> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Matrix3 const& mat, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}]", mat[0], mat[1], mat[2]);
    }
};

template <> struct fmt::formatter<Engine::Matrix3d> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Matrix3d const& mat, FormatContext& ctx) {
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

template <> struct fmt::formatter<Engine::Matrix4d> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Matrix4d const& mat, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}, {}]", mat[0], mat[1], mat[2], mat[3]);
    }
};

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Engine::Matrix3> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Matrix3& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 9)
                throw msgpack::type_error();
            auto dst = const_cast<float*>(&v[0].x);
            for (auto i = 0; i < 9; i++) {
                dst[i] = o.via.array.ptr[i].as<float>();
            }
            return o;
        }
    };
    template <> struct pack<Engine::Matrix3> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Matrix3 const& v) const {
            o.pack_array(9);
            const auto src = &v[0].x;
            for (auto i = 0; i < 9; i++) {
                o.pack_float(src[i]);
            }
            return o;
            ;
        }
    };

    template <> struct convert<Engine::Matrix3d> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Matrix3d& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 9)
                throw msgpack::type_error();
            auto dst = const_cast<double*>(&v[0].x);
            for (auto i = 0; i < 9; i++) {
                dst[i] = o.via.array.ptr[i].as<double>();
            }
            return o;
        }
    };
    template <> struct pack<Engine::Matrix3d> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Matrix3d const& v) const {
            o.pack_array(9);
            const auto src = &v[0].x;
            for (auto i = 0; i < 9; i++) {
                o.pack_float(src[i]);
            }
            return o;
            ;
        }
    };

    template <> struct convert<Engine::Matrix4> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Matrix4& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 16)
                throw msgpack::type_error();
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

    template <> struct convert<Engine::Matrix4d> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Matrix4d& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 16)
                throw msgpack::type_error();
            auto dst = const_cast<double*>(&v[0].x);
            for (auto i = 0; i < 16; i++) {
                dst[i] = o.via.array.ptr[i].as<double>();
            }
            return o;
        }
    };
    template <> struct pack<Engine::Matrix4d> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Matrix4d const& v) const {
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
