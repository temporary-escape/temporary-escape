#pragma once

#include <fmt/format.h>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
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

inline Color4 hsvToRgb(const Vector4& in) {
    float hh, p, q, t, ff;
    long i;
    Color4 out;
    out.w = in.w;

    if (in.y <= 0.0f) { // < is bogus, just shuts up warnings
        out.r = in.z;
        out.g = in.z;
        out.b = in.z;
        return out;
    }
    hh = in.r;
    if (hh >= 360.0f)
        hh = 0.0f;
    hh /= 60.0f;
    i = (long)hh;
    ff = hh - i;
    p = in.z * (1.0f - in.y);
    q = in.z * (1.0f - (in.y * ff));
    t = in.z * (1.0f - (in.y * (1.0f - ff)));

    switch (i) {
    case 0:
        out.r = in.z;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.z;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.z;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.z;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.z;
        break;
    case 5:
    default:
        out.r = in.z;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}
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
