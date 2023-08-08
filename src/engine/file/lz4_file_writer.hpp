#pragma once

#include "../utils/path.hpp"
#include <fstream>

namespace Engine {
class ENGINE_API Lz4FileWriter {
public:
    explicit Lz4FileWriter(const Path& path);
    Lz4FileWriter(const Lz4FileWriter& other);
    Lz4FileWriter(Lz4FileWriter&& other);
    virtual ~Lz4FileWriter();
    Lz4FileWriter& operator=(const Lz4FileWriter& other) = delete;
    Lz4FileWriter& operator=(Lz4FileWriter&& other) = default;

    void write(const char* src, size_t length);
    void close();

private:
    void flush();
    void end();

    struct LZ4;
    std::unique_ptr<LZ4> lz4;
    std::fstream file;
    std::vector<uint8_t> temp;
    std::vector<uint8_t> buff;
    size_t offset{0};
};
} // namespace Engine
