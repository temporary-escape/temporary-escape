#pragma once

#include "../Library.hpp"
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
