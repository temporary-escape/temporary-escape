#pragma once

#include <Engine/Utils/Path.hpp>
#include <fstream>
#include <random>

namespace Engine {
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

inline Path asFile(const std::string& contents) {
    auto tmpPath = std::filesystem::temp_directory_path();
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<uint64_t> dist;

    while (true) {
        auto test = tmpPath / Path(std::to_string(dist(rng)));
        if (!std::filesystem::exists(test)) {
            std::fstream file(test, std::ios::out);
            file << contents;
            return test;
        }
    }
}

inline Path tmpEmptyFile() {
    auto tmpPath = std::filesystem::temp_directory_path();
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<uint64_t> dist;

    while (true) {
        auto test = tmpPath / Path(std::to_string(dist(rng)));
        if (!std::filesystem::exists(test)) {
            std::fstream file(test, std::ios::out);
            return test;
        }
    }
}

} // namespace Engine
