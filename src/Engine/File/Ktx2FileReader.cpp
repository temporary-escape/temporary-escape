extern "C" {
#include <ad_mipmap.h>
}

// clang-format off
#include <volk.h>
#include <ktx.h>
#include <ktxvulkan.h>
// clang-format on

#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "../Utils/StringUtils.hpp"
#include "Ktx2FileReader.hpp"
#include "PngFileReader.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static bool isPowerOfTwo(const uint32_t x) {
    return (x & (x - 1)) == 0;
}

static const char* ktxErrorToStr(const ktx_error_code_e err) {
    switch (err) {
    case KTX_SUCCESS:
        return "Operation was successful";
    case KTX_FILE_DATA_ERROR:
        return "The data in the file is inconsistent with the spec";
    case KTX_FILE_ISPIPE:
        return "The file is a pipe or named pipe";
    case KTX_FILE_OPEN_FAILED:
        return "The target file could not be opened";
    case KTX_FILE_OVERFLOW:
        return "The operation would exceed the max file size";
    case KTX_FILE_READ_ERROR:
        return "An error occurred while reading from the file";
    case KTX_FILE_SEEK_ERROR:
        return "An error occurred while seeking in the file";
    case KTX_FILE_UNEXPECTED_EOF:
        return "File does not have enough data to satisfy request";
    case KTX_FILE_WRITE_ERROR:
        return "An error occurred while writing to the file";
    case KTX_GL_ERROR:
        return "GL operations resulted in an error";
    case KTX_INVALID_OPERATION:
        return "The operation is not allowed in the current state";
    case KTX_INVALID_VALUE:
        return "A parameter value was not valid ";
    case KTX_NOT_FOUND:
        return "Requested key was not found";
    case KTX_OUT_OF_MEMORY:
        return "Not enough memory to complete the operation";
    case KTX_TRANSCODE_FAILED:
        return "Transcoding of block compressed texture failed";
    case KTX_UNKNOWN_FILE_FORMAT:
        return "The file not a KTX file";
    case KTX_UNSUPPORTED_TEXTURE_TYPE:
        return "The KTX file specifies an unsupported texture type";
    case KTX_UNSUPPORTED_FEATURE:
        return "Feature not included in in-use library or not yet implemented";
    case KTX_LIBRARY_NOT_LINKED:
        return "Library dependency (OpenGL or Vulkan) not linked into application";
    default:
        return "Unknown";
    };
}

static KTX_error_code ktx2FstreamRead(ktxStream* str, void* dst, const ktx_size_t count) {
    auto& file = *reinterpret_cast<std::fstream*>(str->data.custom_ptr.address);
    file.read(static_cast<char*>(dst), count);
    return KTX_SUCCESS;
}

KTX_error_code ktx2FstreamSkip(ktxStream* str, const ktx_size_t count) {
    auto& file = *reinterpret_cast<std::fstream*>(str->data.custom_ptr.address);
    file.ignore(count);
    return KTX_SUCCESS;
}

KTX_error_code ktx2FstreamWrite(ktxStream* str, const void* src, const ktx_size_t size, const ktx_size_t count) {
    auto& file = *reinterpret_cast<std::fstream*>(str->data.custom_ptr.address);
    file.write(static_cast<const char*>(src), size * count);
    return KTX_SUCCESS;
}

KTX_error_code ktx2FstreamGetPos(ktxStream* str, ktx_off_t* const offset) {
    auto& file = *reinterpret_cast<std::fstream*>(str->data.custom_ptr.address);
    *offset = file.tellg();
    return KTX_SUCCESS;
}

KTX_error_code ktx2FstreamSetPos(ktxStream* str, const ktx_off_t offset) {
    auto& file = *reinterpret_cast<std::fstream*>(str->data.custom_ptr.address);
    file.seekg(offset, std::ios::beg);
    return KTX_SUCCESS;
}

KTX_error_code ktx2FstreamGetSize(ktxStream* str, ktx_size_t* const size) {
    auto& file = *reinterpret_cast<std::fstream*>(str->data.custom_ptr.address);
    const auto cur = file.tellg();
    file.seekg(0, std::ios::beg);
    *size = file.tellg();
    file.seekg(0, std::ios::end);
    return KTX_SUCCESS;
}

void ktx2FstreamDestruct(ktxStream* str) {
    (void)str;
}

ktxStream ktx2Fstream(std::fstream& file) {
    ktxStream stream;
    stream.type = streamType::eStreamTypeCustom;
    stream.data.custom_ptr.address = &file;
    stream.read = &ktx2FstreamRead;
    stream.skip = &ktx2FstreamSkip;
    stream.write = &ktx2FstreamWrite;
    stream.getpos = &ktx2FstreamGetPos;
    stream.setpos = &ktx2FstreamSetPos;
    stream.getsize = &ktx2FstreamGetSize;
    stream.destruct = &ktx2FstreamDestruct;
    stream.closeOnDestruct = KTX_FALSE;
    return stream;
}

Ktx2FileReader::Ktx2FileReader(const Path& path) : stream{std::make_unique<ktxStream>()} {

    file = std::fstream{path, std::ios::binary | std::ios::in};
    if (!file) {
        EXCEPTION("Failed to open ktx2 file: {}", path);
    }

    *stream = ktx2Fstream(file);

    ktxTexture2* ptr;
    const auto result =
        ktxTexture_CreateFromStream(stream.get(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)(&ptr));

    if (result != KTX_SUCCESS) {
        EXCEPTION("Failed to read ktx2 file: {} error: {}", path, ktxErrorToStr(result));
    }

    ktx = std::shared_ptr<ktxTexture2>{ptr, [](ktxTexture2* p) { ktxTexture_Destroy(ktxTexture(p)); }};

    if (ktx->classId != ktxTexture2_c) {
        EXCEPTION("Failed to read ktx2 file: {} error: file is in ktx1 format but expected ktx2 format");
    }
}

Ktx2FileReader::Ktx2FileReader(const Span<uint8_t>& data) {
    ktxTexture2* ptr;
    const auto result = ktxTexture_CreateFromMemory(
        data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)(&ptr));

    if (result != KTX_SUCCESS) {
        EXCEPTION("Failed to read ktx2 from memory error: {}", ktxErrorToStr(result));
    }

    ktx = std::shared_ptr<ktxTexture2>{ptr, [](ktxTexture2* p) { ktxTexture_Destroy(ktxTexture(p)); }};

    if (ktx->classId != ktxTexture2_c) {
        EXCEPTION("Failed to read ktx2 file: {} error: file is in ktx1 format but expected ktx2 format");
    }
}

Ktx2FileReader::~Ktx2FileReader() = default;

static ktx_transcode_fmt_e getTranscodeTargetETC(Ktx2CompressionTarget target) {
    switch (target) {
    case Ktx2CompressionTarget::R: {
        return KTX_TTF_ETC1_RGB;
    }
    case Ktx2CompressionTarget::RG: {
        return KTX_TTF_ETC1_RGB;
    }
    case Ktx2CompressionTarget::RGB: {
        return KTX_TTF_ETC1_RGB;
    }
    case Ktx2CompressionTarget::RGBA: {
        return KTX_TTF_ETC2_RGBA;
    }
    default: {
        EXCEPTION("Unknown compression target");
    }
    }
}

static ktx_transcode_fmt_e getTranscodeTargetBC(Ktx2CompressionTarget target) {
    switch (target) {
    case Ktx2CompressionTarget::R: {
        return KTX_TTF_BC4_R;
    }
    case Ktx2CompressionTarget::RG: {
        return KTX_TTF_BC5_RG;
    }
    case Ktx2CompressionTarget::RGB: {
        return KTX_TTF_BC1_RGB;
    }
    case Ktx2CompressionTarget::RGBA: {
        return KTX_TTF_BC3_RGBA;
    }
    default:{
        EXCEPTION("Unknown compression target");
    }
    }
}

static ktx_transcode_fmt_e getTranscodeTargetDefault(Ktx2CompressionTarget target) {
    (void)target;
    return KTX_TTF_RGBA32;
}

static ktx_transcode_fmt_e getTranscodeTarget(VulkanCompressionType type, Ktx2CompressionTarget target) {
    switch (type) {
    case VulkanCompressionType::BC3: {
        return getTranscodeTargetBC(target);
    }
    case VulkanCompressionType::ETC2: {
        return getTranscodeTargetETC(target);
    }
    default: {
        return getTranscodeTargetDefault(target);
    }
    }
}

bool Ktx2FileReader::needsTranscoding() const {
    return ktxTexture2_NeedsTranscoding(ktx.get()) == KTX_TRUE;
}

void Ktx2FileReader::transcode(VulkanCompressionType type, Ktx2CompressionTarget target) {
    if (needsTranscoding()) {
        const auto tf = getTranscodeTarget(type, target);

        const auto result = ktxTexture2_TranscodeBasis(ktx.get(), tf, 0);
        if (result != KTX_SUCCESS) {
            EXCEPTION("Failed to transcode ktx2 texture error: {}", ktxErrorToStr(result));
        }
    }
}

VkFormat Ktx2FileReader::getFormat() const {
    return ktxTexture_GetVkFormat(ktxTexture(ktx.get()));
}

KTX_error_code iterateLevelFacesCallback(int level, int layer, int width, int height, int depth,
                                         ktx_uint64_t faceLodSize, void* pixels, void* userdata) {
    auto& chunks = *reinterpret_cast<std::list<Ktx2Chunk>*>(userdata);
    chunks.emplace_back();
    auto& chunk = chunks.back();

    chunk.level = level;
    chunk.layer = layer;
    chunk.size = {width, height, depth};
    chunk.pixels.resize(faceLodSize);
    std::memcpy(chunk.pixels.data(), pixels, chunk.pixels.size());

    return KTX_SUCCESS;
}

void Ktx2FileReader::readData() {
    ktxTexture_IterateLevelFaces(reinterpret_cast<ktxTexture*>(ktx.get()), &iterateLevelFacesCallback, &chunks);
}

const Ktx2Chunk& Ktx2FileReader::getData(const int level, const int layer) {
    for (const auto& chunk : chunks) {
        if (chunk.level == level && chunk.layer == layer) {
            return chunk;
        }
    }
    EXCEPTION("Texture has no such level: {} layer: {}", level, layer);
}

Vector3i Ktx2FileReader::getSize() const {
    return {
        ktx->baseWidth,
        ktx->baseHeight,
        ktx->baseDepth,
    };
}

uint32_t Ktx2FileReader::getMipMapsCount() const {
    return ktx->numLevels;
}

uint32_t Ktx2FileReader::getLayersCount() const {
    return ktx->numLayers * ktx->numFaces;
}

static bool shouldDoMipMaps(const Vector3i& size) {
    return isPowerOfTwo(size.x) && isPowerOfTwo(size.y) && size.z == 1;
}

Ktx2FileWriter::Ktx2FileWriter(const Path& path, const Vector3i& size, VkFormat format, const bool basis) :
    stream{std::make_unique<ktxStream>()}, size{size}, format{format}, basis{basis}, levels{1} {

    if (format != VK_FORMAT_R8G8B8_UNORM && format != VK_FORMAT_R8G8B8A8_UNORM) {
        EXCEPTION("Failed to open ktx2 file: {} error: format can be either RGB8 or RGBA8", path);
    }

    file = std::fstream{path, std::ios::binary | std::ios::out};
    if (!file) {
        EXCEPTION("Failed to open ktx2 file: {}", path);
    }

    *stream = ktx2Fstream(file);

    ktxTextureCreateInfo createInfo{};

    createInfo.vkFormat = format;
    createInfo.baseWidth = size.x;
    createInfo.baseHeight = size.y;
    createInfo.baseDepth = size.z;
    createInfo.numDimensions = 2; // Why?
    createInfo.numLevels = 1 /*log2(createInfo.baseWidth) + 1*/;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;

    if (shouldDoMipMaps(size)) {
        createInfo.numLevels = static_cast<ktx_uint32_t>(log2(createInfo.baseWidth)) + 1;
        levels = createInfo.numLevels;
    }

    ktxTexture2* ptr;
    const auto result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ptr);
    if (result != KTX_SUCCESS) {
        EXCEPTION("Failed to create ktx2 texture, error: {}", ktxErrorToStr(result));
    }

    ktx = std::shared_ptr<ktxTexture2>{ptr, [](ktxTexture2* p) { ktxTexture_Destroy(ktxTexture(p)); }};
}

Ktx2FileWriter::~Ktx2FileWriter() = default;

void Ktx2FileWriter::writeData(const void* src, const bool isNormalMap) {
    writeLevel(src, 0, size);

    if (levels > 1) {
        AdmBitmap bitmap;
        bitmap.width = size.x;
        bitmap.height = size.y;
        bitmap.bytes_per_pixel = format == VK_FORMAT_R8G8B8_UNORM ? 3 : 4;
        bitmap.pixels = const_cast<void*>(src);

        int total{0};
        auto* mipmaps = adm_generate_mipmaps_no_base(&total, &bitmap);
        std::shared_ptr<AdmBitmap> deleter{mipmaps, [total](AdmBitmap* p) { adm_free_mipmaps(p, total); }};

        if (total + 1 != levels) {
            EXCEPTION("Failed to generate connect number of mipmaps");
        }

        for (uint32_t i = 0; i < levels - 1; i++) {
            writeLevel(mipmaps[i].pixels, i + 1, {mipmaps[i].width, mipmaps[i].height, 1});
        }
    }

    compress(isNormalMap);
    writeToFile();
}

void Ktx2FileWriter::writeLevel(const void* src, const uint32_t level, const Vector3i& levelSize) {
    VkExtent3D extent{
        static_cast<uint32_t>(levelSize.x), static_cast<uint32_t>(levelSize.y), static_cast<uint32_t>(levelSize.z)};
    const auto srcSize = getFormatDataSize(format, extent);

    auto result = ktxTexture_SetImageFromMemory(
        ktxTexture(ktx.get()), level, 0, 0, static_cast<const ktx_uint8_t*>(src), srcSize);
    if (result != KTX_SUCCESS) {
        EXCEPTION("Failed to write ktx2 texture, error: {}", ktxErrorToStr(result));
    }
}

void Ktx2FileWriter::compress(const bool isNormalMap) {
    static const int quality = 80;

    ktxBasisParams params{};
    params.structSize = sizeof(ktxBasisParams);
    params.threadCount = 4;
    params.qualityLevel = std::max((quality * 254) / 99 + 1, 1);
    params.compressionLevel = KTX_ETC1S_DEFAULT_COMPRESSION_LEVEL;
    params.uastc = KTX_FALSE;
    params.normalMap = isNormalMap ? KTX_TRUE : KTX_FALSE;

    if (basis) {
        const auto result = ktxTexture2_CompressBasisEx(ktx.get(), &params);
        if (result != KTX_SUCCESS) {
            EXCEPTION("Failed to compress ktx2 texture, error: {}", ktxErrorToStr(result));
        }
    }
}

void Ktx2FileWriter::writeToFile() {
    const auto result = ktxTexture_WriteToStream(reinterpret_cast<ktxTexture*>(ktx.get()), stream.get());
    if (result != KTX_SUCCESS) {
        EXCEPTION("Failed to write ktx2 texture, error: {}", ktxErrorToStr(result));
    }
}

void Engine::ktxCompressFile(const Path& src, const Path& dst, const bool basis) {
    logger.info("Compressing file: {} to {}", src, dst);

    try {
        if (src.extension().string() != ".png") {
            EXCEPTION("only PNG files are supported", src);
        }

        PngFileReader image{src};

        if (image.getFormat() != VK_FORMAT_R8G8B8A8_UNORM && image.getFormat() != VK_FORMAT_R8G8B8_UNORM) {
            EXCEPTION("Only RGBA8 and RGB8 textures are supported for compression");
        }

        Ktx2FileWriter ktx{dst, {image.getSize().x, image.getSize().y, 1}, image.getFormat(), basis};
        ktx.writeData(image.getData(), endsWith(src.stem().string(), "_norm"));
    } catch (...) {
        EXCEPTION_NESTED("Failed to compress texture: {}", src);
    }
}
