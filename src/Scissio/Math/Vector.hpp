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
