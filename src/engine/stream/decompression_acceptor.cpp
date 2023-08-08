#include "decompression_acceptor.hpp"
#include "../utils/exceptions.hpp"
#include <cstring>
#include <lz4.h>

using namespace Engine;

struct DecompressionAcceptor::LZ4 {
    LZ4() {
        LZ4_setStreamDecode(lz4StreamDecode, nullptr, 0);
    }

    LZ4_streamDecode_t lz4StreamDecodeBody{};
    LZ4_streamDecode_t* lz4StreamDecode = &lz4StreamDecodeBody;
};

DecompressionAcceptor::DecompressionAcceptor(const size_t blockBytes) :
    lz4{std::make_unique<LZ4>()}, idx{0}, offset{0}, readCount{0}, decBuf{nullptr, nullptr} {
    raw.resize(blockBytes * 2);
    decBuf[0] = raw.data();
    decBuf[1] = raw.data() + blockBytes;
    cmpBuf.resize(LZ4_COMPRESSBOUND(blockBytes));
}

DecompressionAcceptor::~DecompressionAcceptor() = default;

void DecompressionAcceptor::accept(const char* src, size_t length) {
    while (length > 0) {
        if (offset < sizeof(uint32_t)) {
            auto readToCopy = std::min(length, sizeof(uint32_t));
            if (offset + readToCopy > sizeof(uint32_t)) {
                readToCopy = sizeof(uint32_t) - offset;
            }

            auto* dst = reinterpret_cast<char*>(&readCount) + offset;
            std::memcpy(dst, src, readToCopy);

            length -= readToCopy;
            offset += readToCopy;
            src += readToCopy;
        }

        if (length == 0) {
            break;
        }

        const auto dst = cmpBuf.data() + offset - sizeof(uint32_t);
        const auto toRead = std::min(static_cast<size_t>(readCount), length);

        if (toRead > LZ4_COMPRESSBOUND(raw.size() / 2)) {
            EXCEPTION("Decompress block is too large, to read: {} bytes", toRead);
        }

        std::memcpy(dst, src, toRead);

        length -= toRead;
        offset += toRead;
        src += toRead;

        if (offset == readCount + sizeof(uint32_t)) {
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

    if (decBytes > 0) {
        writeDecompressed(decBuf[idx], decBytes);
        /*unp.reserve_buffer(decBytes);
        std::memcpy(unp.buffer(), decBuf[idx], decBytes);
        unp.buffer_consumed(decBytes);

        auto oh = std::make_shared<msgpack::object_handle>();
        while (unp.next(*oh)) {
            receiveObject(std::move(oh));
            oh = std::make_shared<msgpack::object_handle>();
        }*/
    }

    idx = (idx + 1) % 2;
}
