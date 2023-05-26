#pragma once

#include "../library.hpp"
#include <fmt/format.h>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <msgpack.hpp>
#include <ostream>

namespace Engine {
using Vector2 = glm::vec2;
using Vector2i = glm::i32vec2;
using Vector3 = glm::vec3;
using Vector3i = glm::i32vec3;
using Vector4 = glm::vec4;
using Vector4i = glm::i32vec4;
using Color4 = glm::vec4;

inline Color4 alpha(float a) {
    return Color4{1.0f, 1.0f, 1.0f, a};
}

ENGINE_API void bindMathVectors(Lua& lua);
} // namespace Engine

template <> struct fmt::formatter<Engine::Vector2> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Vector2 const& vec, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}]", vec.x, vec.y);
    }
};

template <> struct fmt::formatter<Engine::Vector2i> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Vector2i const& vec, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}]", vec.x, vec.y);
    }
};

template <> struct fmt::formatter<Engine::Vector3> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Vector3 const& vec, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);
    }
};

template <> struct fmt::formatter<Engine::Vector3i> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Vector3i const& vec, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);
    }
};

template <> struct fmt::formatter<Engine::Vector4> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Vector4 const& vec, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w);
    }
};

template <> struct fmt::formatter<Engine::Vector4i> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Vector4i const& vec, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w);
    }
};

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Engine::Vector2> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector2& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector2> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector2 const& v) const {
            o.pack_array(2);
            o.pack_float(v.x);
            o.pack_float(v.y);
            return o;
        }
    };

    template <> struct convert<Engine::Vector2i> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector2i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector2i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector2i const& v) const {
            o.pack_array(2);
            o.pack_int(v.x);
            o.pack_int(v.y);
            return o;
        }
    };

    template <> struct convert<Engine::Vector3> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector3& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector3> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector3 const& v) const {
            o.pack_array(3);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            return o;
        }
    };

    template <> struct convert<Engine::Vector3i> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector3i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>(), o.via.array.ptr[2].as<int>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector3i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector3i const& v) const {
            o.pack_array(3);
            o.pack_int(v.x);
            o.pack_int(v.y);
            o.pack_int(v.z);
            return o;
        }
    };

    template <> struct convert<Engine::Vector4> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector4& v) const {
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
    template <> struct pack<Engine::Vector4> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector4 const& v) const {
            o.pack_array(4);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            o.pack_float(v.w);
            return o;
        }
    };

    template <> struct convert<Engine::Vector4i> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector4i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(),
                 o.via.array.ptr[1].as<int>(),
                 o.via.array.ptr[2].as<int>(),
                 o.via.array.ptr[3].as<int>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector4i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector4i const& v) const {
            o.pack_array(4);
            o.pack_int(v.x);
            o.pack_int(v.y);
            o.pack_int(v.z);
            o.pack_int(v.w);
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
