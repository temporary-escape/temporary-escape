#pragma once

#include "../Library.hpp"
#include "../Utils/MoveableCopyable.hpp"
#include <array>
#include <memory>

namespace Engine {
/**
 * Decompression acceptor that produces decompressed data
 */
class ENGINE_API DecompressionAcceptor {
public:
    static constexpr size_t blockBytes = 1024 * 8;
    static constexpr size_t blockBytesCompressed = 8240;

    /**
     * @param blockBytes Maximum number of bytes per each compressed block.
     * @warning The blockBytes must match the compression stream's blockBytes!
     */
    explicit DecompressionAcceptor();
    ~DecompressionAcceptor();
    NON_COPYABLE(DecompressionAcceptor);
    DecompressionAcceptor(DecompressionAcceptor&& other);
    DecompressionAcceptor& operator=(DecompressionAcceptor&& other);

    /**
     * Accept arbitrary number of bytes.
     * @param data The compressed source data.
     * @param length The length of the source data.
     */
    void accept(const char* data, size_t length);

protected:
    /**
     * Called each time there is some data in the decompressed stream.
     */
    virtual void writeDecompressed(const char* data, size_t length) = 0;

private:
    void decompress();

    struct LZ4;
    std::unique_ptr<LZ4> lz4;
    std::array<char, blockBytes * 2> raw;
    char* decBuf[2];
    std::array<char, blockBytesCompressed> cmpBuf;
    size_t idx;
    size_t offset;
    uint32_t readCount;
};
} // namespace Engine
