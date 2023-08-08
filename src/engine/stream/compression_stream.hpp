#pragma once

#include "../library.hpp"
#include "../utils/moveable_copyable.hpp"
#include <memory>
#include <vector>

namespace Engine {

/**
 * Compression stream that can be used with Msgpack.
 * It produces compressed buffers via sendBuffer().
 * You must call flush() after each message end.
 */
class ENGINE_API CompressionStream {
public:
    /**
     * @param blockBytes Maximum number of bytes per each compressed block.
     */
    explicit CompressionStream(size_t blockBytes = 1024 * 8);
    virtual ~CompressionStream();
    NON_COPYABLE(CompressionStream);
    CompressionStream(CompressionStream&& other);
    CompressionStream& operator=(CompressionStream&& other);

    /**
     * Write arbitrary number of bytes into the stream.
     *
     * @param src The source data.
     * @param length Length of the source data.
     */
    void write(const char* src, size_t length);

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
    virtual void writeCompressed(std::shared_ptr<std::vector<char>> buffer) = 0;

private:
    struct LZ4;
    std::unique_ptr<LZ4> lz4;
    std::vector<char> raw;
    char* buffers[2];
    size_t idx;
    size_t offset;
};
} // namespace Engine
