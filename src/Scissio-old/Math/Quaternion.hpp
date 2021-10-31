#pragma once

#include "../Library.hpp"
#include <glm/gtx/quaternion.hpp>
#include <ostream>
#include <random>

namespace Scissio {
using Quaternion = glm::quat;

SCISSIO_API Quaternion randomQuaternion(std::mt19937_64& rng);
} // namespace Scissio

inline std::ostream& operator<<(std::ostream& os, Scissio::Quaternion const& v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
    return os;
}
