#include "compression_stream.hpp"
#include <cstring>
#include <lz4.h>

using namespace Engine;

struct CompressionStream::LZ4 {
    LZ4() {
        LZ4_initStream(lz4Stream, sizeof(*lz4Stream));
    }

    LZ4_stream_t lz4StreamBody{};
    LZ4_stream_t* lz4Stream = &lz4StreamBody;
};

CompressionStream::CompressionStream(const size_t blockBytes) :
    lz4{std::make_unique<LZ4>()}, idx{0}, offset{0}, buffers{nullptr, nullptr} {
    raw.resize(blockBytes * 2);
    buffers[0] = raw.data();
    buffers[1] = raw.data() + blockBytes;
}

CompressionStream::~CompressionStream() = default;

void CompressionStream::write(const char* src, size_t length) {
    while (length > 0) {
        if (offset + length > raw.size() / 2) {
            flush();
        }

        const auto toWrite = std::min(raw.size() / 2 - offset, length);
        if (toWrite == 1) {
            buffers[idx][offset] = *src;
        } else {
            std::memcpy(buffers[idx] + offset, src, toWrite);
        }

        offset += toWrite;
        length -= toWrite;
        src += toWrite;
    }
}

void CompressionStream::flush() {
    auto compressed = std::make_shared<std::vector<char>>();
    const auto compressBound = LZ4_COMPRESSBOUND(raw.size() / 2);

    compressed->resize(sizeof(uint32_t) + compressBound);
    const auto cmpBuf = compressed->data() + sizeof(uint32_t);

    const uint32_t cmpBytes = LZ4_compress_fast_continue(lz4->lz4Stream,                  // Stream
                                                         buffers[idx],                    // Source uncompressed data
                                                         cmpBuf,                          // Destination compressed data
                                                         static_cast<int>(offset),        // Source data length
                                                         static_cast<int>(compressBound), // Destination buffer length
                                                         1);

    // Reset for the next iteration
    offset = 0;
    idx = (idx + 1) % 2;

    if (cmpBytes == 0) {
        return;
    }

    // Resize the target buffer
    compressed->resize(cmpBytes + sizeof(uint32_t));

    // Write the data length (needed for decompression)
    std::memcpy(compressed->data(), &cmpBytes, sizeof(cmpBytes));

    // Send out the buffer
    writeCompressed(std::move(compressed));
}
