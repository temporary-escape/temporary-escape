#pragma once

#include "path.hpp"
#include <fstream>
#include <msgpack.hpp>

namespace Engine {
class Lz4FileWriter {
public:
    explicit Lz4FileWriter(const Path& path);
    Lz4FileWriter(const Lz4FileWriter& other) = delete;
    Lz4FileWriter(Lz4FileWriter&& other) = default;
    virtual ~Lz4FileWriter();
    Lz4FileWriter& operator=(const Lz4FileWriter& other) = delete;
    Lz4FileWriter& operator=(Lz4FileWriter&& other) = default;

    void write(const char* src, size_t length);
    void close();

    template <typename T> void pack(const T& value) {
        msgpack::packer<Lz4FileWriter> packer{*this};
        packer.pack(value);
    }

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

class Lz4FileReader {
public:
    explicit Lz4FileReader(const Path& path);
    Lz4FileReader(const Lz4FileReader& other) = delete;
    Lz4FileReader(Lz4FileReader&& other) = default;
    virtual ~Lz4FileReader();
    Lz4FileReader& operator=(const Lz4FileReader& other) = delete;
    Lz4FileReader& operator=(Lz4FileReader&& other) = default;

    size_t read(char* dst, size_t length);
    bool read(msgpack::object_handle& obj);

    bool eof() const {
        return file.eof() && !compressSize && offset >= decompressSize;
    }

    template <typename T> bool unpack(T& value) {
        msgpack::object_handle result;
        if (read(result)) {
            msgpack::object obj{result.get()};
            obj.convert(value);
            return true;
        }

        return false;
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
    msgpack::unpacker unp;
};
} // namespace Engine
