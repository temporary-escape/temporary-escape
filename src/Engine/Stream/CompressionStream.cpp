#include "CompressionStream.hpp"
#include <cstring>
#include <lz4.h>

using namespace Engine;

static_assert(CompressionStream::blockBytesCompressed == LZ4_COMPRESSBOUND(CompressionStream::blockBytes));

struct CompressionStream::LZ4 {
    LZ4() {
        LZ4_initStream(lz4Stream, sizeof(*lz4Stream));
    }

    LZ4_stream_t lz4StreamBody{};
    LZ4_stream_t* lz4Stream = &lz4StreamBody;
};

CompressionStream::CompressionStream() : lz4{std::make_unique<LZ4>()}, idx{0}, offset{0}, buffers{nullptr, nullptr} {
    buffers[0] = raw.data();
    buffers[1] = raw.data() + blockBytes;
}

CompressionStream::~CompressionStream() = default;

CompressionStream::CompressionStream(CompressionStream&& other) = default;

CompressionStream& CompressionStream::operator=(CompressionStream&& other) = default;

void CompressionStream::write(const char* data, size_t length) {
    while (length > 0) {
        if (offset + length > blockBytes) {
            flush();
        }

        const auto toWrite = std::min(blockBytes - offset, length);
        if (toWrite == 1) {
            buffers[idx][offset] = *data;
        } else {
            std::memcpy(buffers[idx] + offset, data, toWrite);
        }

        offset += toWrite;
        length -= toWrite;
        data += toWrite;
    }
}

void CompressionStream::flush() {
    const auto cmpBuf = compressed.data() + sizeof(uint32_t);

    const uint32_t cmpBytes =
        LZ4_compress_fast_continue(lz4->lz4Stream,                         // Stream
                                   buffers[idx],                           // Source uncompressed data
                                   cmpBuf,                                 // Destination compressed data
                                   static_cast<int>(offset),               // Source data length
                                   static_cast<int>(blockBytesCompressed), // Destination buffer length
                                   1);

    // Reset for the next iteration
    offset = 0;
    idx = (idx + 1) % 2;

    if (cmpBytes == 0) {
        return;
    }

    // Write the data length (needed for decompression)
    std::memcpy(compressed.data(), &cmpBytes, sizeof(uint32_t));

    // Send out the buffer
    writeCompressed(compressed.data(), cmpBytes + sizeof(uint32_t));
}
