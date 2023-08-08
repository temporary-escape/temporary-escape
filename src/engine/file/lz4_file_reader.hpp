#pragma once

#include "../utils/path.hpp"
#include <fstream>

namespace Engine {
class ENGINE_API Lz4FileReader {
public:
    explicit Lz4FileReader(const Path& path);
    Lz4FileReader(const Lz4FileReader& other);
    Lz4FileReader(Lz4FileReader&& other);
    virtual ~Lz4FileReader();
    Lz4FileReader& operator=(const Lz4FileReader& other) = delete;
    Lz4FileReader& operator=(Lz4FileReader&& other) = default;

    size_t read(char* dst, size_t length);

    bool eof() const {
        return file.eof() && !compressSize && offset >= decompressSize;
    }

private:
    void decompressMore();
    void readMore();

    struct LZ4;
    std::unique_ptr<LZ4> lz4;
    std::fstream file;
    std::vector<uint8_t> temp;
    std::vector<uint8_t> buff;
    size_t offset{0};
    size_t compressSize{0};
    uint8_t* compressPtr{nullptr};
    size_t decompressSize{0};
};
} // namespace Engine
