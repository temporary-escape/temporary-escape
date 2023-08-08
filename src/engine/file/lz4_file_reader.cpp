#include "lz4_file_reader.hpp"
#include "../utils/exceptions.hpp"
#include <lz4frame.h>

using namespace Engine;

static constexpr size_t chunkSize = 4096;
static_assert(chunkSize >= LZ4F_HEADER_SIZE_MAX, "LZ4 header size is larger than file chunk size");

struct Lz4FileReader::LZ4 {
    LZ4() {
        const auto r = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
        if (LZ4F_isError(r)) {
            LZ4F_freeDecompressionContext(ctx);
            EXCEPTION("Failed to initialize LZ4 decompression context error: {}", r);
        }
    }

    ~LZ4() {
        LZ4F_freeDecompressionContext(ctx);
    }

    LZ4F_decompressionContext_t ctx{};
};

static size_t blockSizeIdToBytes(const LZ4F_frameInfo_t& info) {
    switch (info.blockSizeID) {
    case LZ4F_default:
    case LZ4F_max64KB:
        return 64 * 1024;
    case LZ4F_max256KB:
        return 256 * 1024;
    case LZ4F_max1MB:
        return 1 * 1024 * 1024;
    case LZ4F_max4MB:
        return 4 * 1024 * 1024;
    default:
        throw std::invalid_argument("Invalid LZ4 block size");
    }
}

Lz4FileReader::Lz4FileReader(const Path& path) : lz4{std::make_unique<LZ4>()} {
    file = std::fstream{path, std::ios::binary | std::ios::in};
    if (!file) {
        EXCEPTION("Failed to open file for reading file: {}", path);
    }

    char header[LZ4F_HEADER_SIZE_MAX];
    file.read(header, sizeof(header));
    if (file.gcount() != LZ4F_HEADER_SIZE_MAX) {
        EXCEPTION("Failed to read LZ4 header file: {}", path);
    }

    LZ4F_frameInfo_t info{};
    auto consumedSize = sizeof(header);
    const auto r = LZ4F_getFrameInfo(lz4->ctx, &info, header, &consumedSize);
    if (LZ4F_isError(r)) {
        EXCEPTION("Failed to get LZ4 frame info file: {}", path);
    }

    file.seekg(static_cast<std::streamsize>(consumedSize), std::ios::beg);

    try {
        buff.resize(blockSizeIdToBytes(info));
        temp.resize(chunkSize);
    } catch (std::exception& e) {
        EXCEPTION("Failed to get LZ4 block size file: {}", path);
    }
}

Lz4FileReader::~Lz4FileReader() = default;

size_t Lz4FileReader::read(char* dst, size_t length) {
    auto data = dst;

    while (length > 0) {
        if (offset >= decompressSize) {
            decompressMore();
        }

        if (decompressSize == 0) {
            break;
        }

        const auto toRead = std::min(decompressSize - offset, length);
        std::memcpy(data, temp.data() + offset, toRead);

        length -= toRead;
        data += toRead;
        offset += toRead;
    }

    return data - reinterpret_cast<char*>(dst);
}

void Lz4FileReader::decompressMore() {
    offset = 0;

    if (compressSize == 0) {
        readMore();
    }

    if (!compressPtr || !compressSize) {
        decompressSize = 0;
        return;
    }

    auto srcSize = compressSize;
    decompressSize = temp.size();
    const auto n = LZ4F_decompress(lz4->ctx, temp.data(), &decompressSize, compressPtr, &srcSize, nullptr);
    if (LZ4F_isError(n)) {
        EXCEPTION("Decompression failed error: {}", n);
    }

    compressPtr += srcSize;
    compressSize -= srcSize;
}

void Lz4FileReader::readMore() {
    if (!file.eof()) {
        file.read(reinterpret_cast<char*>(buff.data()), static_cast<std::streamsize>(buff.size()));
        compressSize = static_cast<size_t>(file.gcount());
        compressPtr = buff.data();
    } else {
        compressSize = 0;
        compressPtr = nullptr;
    }
}
