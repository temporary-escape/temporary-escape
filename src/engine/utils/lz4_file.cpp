#include "lz4_file.hpp"
#include "exceptions.hpp"
#include <lz4frame.h>

#define CHUNK_SIZE 4096

static_assert(CHUNK_SIZE >= LZ4F_HEADER_SIZE_MAX, "LZ4 header size is larger than file chunk size");

using namespace Engine;

static const LZ4F_preferences_t kPrefs = {
    {LZ4F_max64KB, LZ4F_blockLinked, LZ4F_noContentChecksum, LZ4F_frame, 0 /* unknown content size */,
     0 /* no dictID */, LZ4F_noBlockChecksum},
    0,         /* compression level; 0 == default */
    0,         /* autoflush */
    0,         /* favor decompression speed */
    {0, 0, 0}, /* reserved, must be set to 0 */
};

struct Lz4FileWriter::LZ4 {
    LZ4() {
        const auto r = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
        if (LZ4F_isError(r)) {
            LZ4F_freeCompressionContext(ctx);
            EXCEPTION("Failed to initialize LZ4 compression context error: {}", r);
        }
    }

    ~LZ4() {
        LZ4F_freeCompressionContext(ctx);
    }

    LZ4F_compressionContext_t ctx{};
};

Lz4FileWriter::Lz4FileWriter(const Path& path) : lz4{std::make_unique<LZ4>()} {
    file = std::fstream{path, std::ios::binary | std::ios::out};
    if (!file) {
        EXCEPTION("Failed to open file for writing file: {}", path);
    }

    const auto frameSize = LZ4F_compressBound(CHUNK_SIZE, &kPrefs);
    buff.resize(frameSize);
    temp.resize(CHUNK_SIZE);

    const auto off = LZ4F_compressBegin(lz4->ctx, buff.data(), buff.size(), &kPrefs);
    if (LZ4F_isError(off)) {
        EXCEPTION("Failed to begin LZ4 compression context error: {} file: {}", off, path);
    }

    file.write(reinterpret_cast<const char*>(buff.data()), static_cast<std::streamsize>(off));
}

Lz4FileWriter::~Lz4FileWriter() {
    close();
}

void Lz4FileWriter::write(const char* src, size_t length) {
    while (length > 0) {
        if (offset >= temp.size()) {
            flush();
        }

        const auto toWrite = std::min(temp.size() - offset, length);
        std::memcpy(temp.data() + offset, src, toWrite);

        offset += toWrite;
        length -= toWrite;
        src += toWrite;
    }
}

void Lz4FileWriter::flush() {
    if (offset) {
        const auto n = LZ4F_compressUpdate(lz4->ctx, buff.data(), buff.size(), temp.data(), offset, nullptr);

        if (LZ4F_isError(n)) {
            EXCEPTION("Compression failed error: {}", n);
        }

        file.write(reinterpret_cast<const char*>(buff.data()), static_cast<std::streamsize>(n));
    }
    offset = 0;
}

void Lz4FileWriter::end() {
    const auto r = LZ4F_compressEnd(lz4->ctx, buff.data(), buff.size(), nullptr);
    file.write(reinterpret_cast<const char*>(buff.data()), static_cast<std::streamsize>(r));
}

void Lz4FileWriter::close() {
    if (lz4) {
        flush();
        end();
        file.close();
    }
    lz4.reset();
}

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
        temp.resize(CHUNK_SIZE);
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

bool Lz4FileReader::read(msgpack::object_handle& obj) {
    if (unp.next(obj)) {
        return true;
    }

    while (!eof()) {
        unp.reserve_buffer(CHUNK_SIZE);

        const auto consumed = read(unp.buffer(), CHUNK_SIZE);
        unp.buffer_consumed(consumed);

        if (unp.next(obj)) {
            return true;
        }
    }

    return false;
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
