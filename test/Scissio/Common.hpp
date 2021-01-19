#pragma once

#include "Math/Quaternion.hpp"
#include "Math/Vector.hpp"

#include <Utils/Exceptions.hpp>
#include <catch2/catch.hpp>
#include <iostream>
#include <sstream>

#define TEST(NAME) TEST_CASE(NAME, __FILENAME__)

using namespace Scissio;

namespace Catch {
template <> struct StringMaker<Vector2> {
    static std::string convert(Vector2 const& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

template <> struct StringMaker<Vector2i> {
    static std::string convert(Vector2i const& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

template <> struct StringMaker<Vector3> {
    static std::string convert(Vector3 const& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

template <> struct StringMaker<Vector3i> {
    static std::string convert(Vector3i const& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

template <> struct StringMaker<Vector4> {
    static std::string convert(Vector4 const& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

template <> struct StringMaker<Vector4i> {
    static std::string convert(Vector4i const& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

template <> struct StringMaker<Quaternion> {
    static std::string convert(Quaternion const& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};
} // namespace Catch
