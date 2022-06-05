#pragma once

#include <TemporaryEscape/Math/Quaternion.hpp>
#include <TemporaryEscape/Math/Vector.hpp>
#include <TemporaryEscape/Network/NetworkAsio.hpp>
#include <TemporaryEscape/Utils/Exceptions.hpp>
#include <TemporaryEscape/Utils/Path.hpp>
#include <catch2/catch.hpp>
#include <chrono>
#include <iostream>
#include <sstream>

using namespace Engine;

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

extern Path asFile(const std::string& contents);
extern Path tmpEmptyFile();

class IoServiceRunner {
public:
    explicit IoServiceRunner(asio::io_service& service) : service(service) {
        work = std::make_unique<asio::io_service::work>(service);
        thread = std::thread([this]() { this->service.run(); });
    }

    ~IoServiceRunner() {
        stop();
    }

    void stop() {
        work.reset();
        thread.join();
    }

    asio::io_service& service;
    std::unique_ptr<asio::io_service::work> work;
    std::thread thread;
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

template <typename T> class RefCounter : public T {
public:
    RefCounter(size_t& counter) : counter(counter) {
        counter++;
    }
    ~RefCounter() {
        counter--;
    }

private:
    size_t& counter;
};

namespace Catch {
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
