#include "Common.hpp"
#include <fstream>

struct Init {
    Init() {
        Log::configure(true);
    }
};

static Init init{};

Path asFile(const std::string& contents) {
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

Path tmpEmptyFile() {
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
