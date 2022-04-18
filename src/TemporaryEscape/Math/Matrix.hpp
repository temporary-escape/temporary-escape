#pragma once

#include "Quaternion.hpp"
#include "Vector.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

namespace Engine {
using Matrix4 = glm::mat4x4;
using Matrix3 = glm::mat3x3;
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
