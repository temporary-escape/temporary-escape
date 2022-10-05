#pragma once

#include "../Library.hpp"
#include <fmt/format.h>
#include <glm/gtx/quaternion.hpp>
#include <ostream>
#include <random>

namespace Engine {
using Quaternion = glm::quat;

ENGINE_API Quaternion randomQuaternion(std::mt19937_64& rng);
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
