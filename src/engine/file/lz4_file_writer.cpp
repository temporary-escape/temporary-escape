#include "lz4_file_writer.hpp"
#include "../utils/exceptions.hpp"
#include <lz4frame.h>

using namespace Engine;

static constexpr size_t chunkSize = 4096;
static_assert(chunkSize >= LZ4F_HEADER_SIZE_MAX, "LZ4 header size is larger than file chunk size");

static const LZ4F_preferences_t kPrefs = {
    {LZ4F_max64KB,
     LZ4F_blockLinked,
     LZ4F_noContentChecksum,
     LZ4F_frame,
     0 /* unknown content size */,
     0 /* no dictID */,
     LZ4F_noBlockChecksum},
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

    const auto frameSize = LZ4F_compressBound(chunkSize, &kPrefs);
    buff.resize(frameSize);
    temp.resize(chunkSize);

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
