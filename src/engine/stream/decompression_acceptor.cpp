#include "decompression_acceptor.hpp"
#include "../utils/exceptions.hpp"
#include <cstring>
#include <lz4.h>

using namespace Engine;

static_assert(DecompressionAcceptor::blockBytesCompressed == LZ4_COMPRESSBOUND(DecompressionAcceptor::blockBytes));

struct DecompressionAcceptor::LZ4 {
    LZ4() {
        LZ4_setStreamDecode(lz4StreamDecode, nullptr, 0);
    }

    LZ4_streamDecode_t lz4StreamDecodeBody{};
    LZ4_streamDecode_t* lz4StreamDecode = &lz4StreamDecodeBody;
};

DecompressionAcceptor::DecompressionAcceptor() :
    lz4{std::make_unique<LZ4>()}, idx{0}, offset{0}, readCount{0}, decBuf{nullptr, nullptr} {
    decBuf[0] = raw.data();
    decBuf[1] = raw.data() + blockBytes;
}

DecompressionAcceptor::~DecompressionAcceptor() = default;

DecompressionAcceptor::DecompressionAcceptor(DecompressionAcceptor&& other) = default;

DecompressionAcceptor& DecompressionAcceptor::operator=(DecompressionAcceptor&& other) = default;

void DecompressionAcceptor::accept(const char* data, size_t length) {
    while (length > 0) {
        if (offset < sizeof(uint32_t)) {
            auto readToCopy = std::min(length, sizeof(uint32_t));
            if (offset + readToCopy > sizeof(uint32_t)) {
                readToCopy = sizeof(uint32_t) - offset;
            }

            auto* dst = reinterpret_cast<char*>(&readCount) + offset;
            std::memcpy(dst, data, readToCopy);

            length -= readToCopy;
            offset += readToCopy;
            data += readToCopy;
        }

        if (length == 0) {
            break;
        }

        const auto cmpBufOffset = offset - sizeof(uint32_t);

        const auto dst = cmpBuf.data() + cmpBufOffset;
        auto toRead = std::min(static_cast<size_t>(readCount - cmpBufOffset), length);

        if (toRead + cmpBufOffset > cmpBuf.size()) {
            EXCEPTION("Decompress block is too large, to read: {} bytes", toRead);
        }

        std::memcpy(dst, data, toRead);

        length -= toRead;
        offset += toRead;
        data += toRead;

        if (offset > sizeof(uint32_t) && offset - sizeof(uint32_t) == readCount) {
            decompress();
            offset = 0;
        }
    }
}

void DecompressionAcceptor::decompress() {
    const int decBytes = LZ4_decompress_safe_continue(lz4->lz4StreamDecode,            // Stream
                                                      cmpBuf.data(),                   // Source compressed data
                                                      decBuf[idx],                     // Destination decompressed data
                                                      static_cast<int>(readCount),     // Number of compressed bytes
                                                      static_cast<int>(raw.size() / 2) // Max size of the destination
    );

    const auto* res = decBuf[idx];
    idx = (idx + 1) % 2;

    if (decBytes > 0) {
        writeDecompressed(res, decBytes);
    }
}
