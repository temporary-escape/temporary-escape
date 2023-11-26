#pragma once

#include "../Library.hpp"
#include "../Utils/MoveableCopyable.hpp"
#include <array>
#include <memory>

namespace Engine {

/**
 * Compression stream that can be used with Msgpack.
 * It produces compressed buffers via sendBuffer().
 * You must call flush() after each message end.
 */
class ENGINE_API CompressionStream {
public:
    static constexpr size_t blockBytes = 1024 * 8;
    static constexpr size_t blockBytesCompressed = 8240;

    /**
     * @param blockBytes Maximum number of bytes per each compressed block.
     */
    CompressionStream();
    virtual ~CompressionStream();
    NON_COPYABLE(CompressionStream);
    CompressionStream(CompressionStream&& other);
    CompressionStream& operator=(CompressionStream&& other);

    /**
     * Write arbitrary number of bytes into the stream.
     *
     * @param data The source data.
     * @param length Length of the source data.
     */
    void write(const char* data, size_t length);

    /**
     * Flush the compressed buffer out. Must be called
     * after each message end.
     */
    void flush();

protected:
    /**
     * The method that gets called every time some buffer needs to be sent out.
     * This will be triggered if the current buffer size reaches "blockBytes"
     * or the flush has been called.
     *
     * @param buffer
     */
    virtual void writeCompressed(const char* data, size_t length) = 0;

private:
    struct LZ4;
    std::unique_ptr<LZ4> lz4;
    std::array<char, blockBytes * 2> raw;
    std::array<char, blockBytesCompressed + sizeof(uint32_t)> compressed;
    char* buffers[2];
    size_t idx;
    size_t offset;
};
} // namespace Engine
