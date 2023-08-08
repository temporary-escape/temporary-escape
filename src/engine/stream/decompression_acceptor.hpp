#pragma once

#include "../library.hpp"
#include "../utils/moveable_copyable.hpp"
#include <memory>
#include <vector>

namespace Engine {
/**
 * Decompression acceptor that produces decompressed data
 */
class ENGINE_API DecompressionAcceptor {
public:
    /**
     * @param blockBytes Maximum number of bytes per each compressed block.
     * @warning The blockBytes must match the compression stream's blockBytes!
     */
    explicit DecompressionAcceptor(size_t blockBytes = 1024 * 8);
    ~DecompressionAcceptor();
    NON_COPYABLE(DecompressionAcceptor);
    DecompressionAcceptor(DecompressionAcceptor&& other);
    DecompressionAcceptor& operator=(DecompressionAcceptor&& other);

    /**
     * Accept arbitrary number of bytes.
     * @param src The compressed source data.
     * @param length The length of the source data.
     */
    void accept(const char* src, size_t length);

protected:
    /**
     * Called each time there is some data in the decompressed stream.
     */
    virtual void writeDecompressed(const void* data, size_t length) = 0;

private:
    void decompress();

    struct LZ4;
    std::unique_ptr<LZ4> lz4;
    std::vector<char> raw;
    char* decBuf[2];
    std::vector<char> cmpBuf;
    // msgpack::unpacker unp;
    size_t idx;
    size_t offset;
    uint32_t readCount;
};
} // namespace Engine
