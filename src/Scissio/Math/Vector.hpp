#pragma once

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <ostream>

namespace Scissio {
using Vector2 = glm::vec2;
using Vector2i = glm::i32vec2;
using Vector3 = glm::vec3;
using Vector3i = glm::i32vec3;
using Vector4 = glm::vec4;
using Vector4i = glm::i32vec4;
using Color4 = glm::vec4;
} // namespace Scissio

inline std::ostream& operator<<(std::ostream& os, Scissio::Vector2 const& v) {
    os << "[" << v.x << ", " << v.y << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Scissio::Vector2i const& v) {
    os << "[" << v.x << ", " << v.y << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Scissio::Vector3 const& v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Scissio::Vector3i const& v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Scissio::Vector4 const& v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Scissio::Vector4i const& v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
    return os;
}
