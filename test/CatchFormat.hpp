#pragma once

#define CATCH_CONFIG_MAIN
#include <Engine/Math/Matrix.hpp>
#include <Engine/Math/Quaternion.hpp>
#include <Engine/Math/Vector.hpp>
#include <catch.hpp>
#include <sstream>

namespace Catch {
using namespace Engine;

template <> struct StringMaker<Vector2> {
    static std::string convert(Vector2 const& value) {
        std::stringstream ss;
        ss << "[" << value.x << ", " << value.y << "]";
        return ss.str();
    }
};

template <> struct StringMaker<Vector2i> {
    static std::string convert(Vector2i const& value) {
        std::stringstream ss;
        ss << "[" << value.x << ", " << value.y << "]";
        return ss.str();
    }
};

template <> struct StringMaker<Vector3> {
    static std::string convert(Vector3 const& value) {
        std::stringstream ss;
        ss << "[" << value.x << ", " << value.y << ", " << value.z << "]";
        return ss.str();
    }
};

template <> struct StringMaker<Vector3i> {
    static std::string convert(Vector3i const& value) {
        std::stringstream ss;
        ss << "[" << value.x << ", " << value.y << ", " << value.z << "]";
        return ss.str();
    }
};

template <> struct StringMaker<Vector4> {
    static std::string convert(Vector4 const& value) {
        std::stringstream ss;
        ss << "[" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "]";
        return ss.str();
    }
};

template <> struct StringMaker<Vector4i> {
    static std::string convert(Vector4i const& value) {
        std::stringstream ss;
        ss << "[" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "]";
        return ss.str();
    }
};

template <> struct StringMaker<Quaternion> {
    static std::string convert(Quaternion const& value) {
        std::stringstream ss;
        ss << "[" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "]";
        return ss.str();
    }
};
} // namespace Catch
