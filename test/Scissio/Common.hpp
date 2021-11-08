#pragma once

#include <Math/Quaternion.hpp>
#include <Math/Vector.hpp>
#include <Utils/Exceptions.hpp>
#include <Utils/Path.hpp>
#include <catch2/catch.hpp>
#include <chrono>
#include <iostream>
#include <sstream>

using namespace Scissio;

class TmpDir {
public:
    TmpDir() {
        auto tmpPath = std::filesystem::temp_directory_path();
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<uint64_t> dist;

        while (true) {
            auto test = tmpPath / Path(std::to_string(dist(rng)));
            if (std::filesystem::create_directory(test)) {
                path = test;
                break;
            }
        }
    }

    ~TmpDir() {
        std::filesystem::remove_all(path);
    }

    [[nodiscard]] const Path& value() const {
        return path;
    }

private:
    Path path;
};

static inline bool waitForCondition(const std::function<bool()>& fn) {
    const auto start = std::chrono::steady_clock::now();

    while (true) {
        if (fn()) {
            return true;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (test > 1000) {
            return false;
        }
    }
}

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
