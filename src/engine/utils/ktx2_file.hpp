#pragma once

#include "../math/vector.hpp"
#include "../vulkan/vulkan_types.hpp"
#include "path.hpp"
#include <fstream>

struct ktxTexture;
struct ktxTexture2;
struct ktxStream;

namespace Engine {
enum class Ktx2CompressionTarget {
    R,
    RG,
    RGB,
    RGBA,
};

struct ENGINE_API Ktx2Chunk {
    Vector3 size;
    int level{0};
    int layer{0};
    std::vector<char> pixels;
};

class ENGINE_API Ktx2FileReader {
public:
    explicit Ktx2FileReader(const Path& path);
    virtual ~Ktx2FileReader();

    [[nodiscard]] bool needsTranscoding() const;
    void transcode(VulkanCompressionType type, Ktx2CompressionTarget target);
    void readData();

    [[nodiscard]] VkFormat getFormat() const;
    [[nodiscard]] const Ktx2Chunk& getData(int level, int layer);
    [[nodiscard]] Vector3i getSize() const;
    [[nodiscard]] uint32_t getMipMapsCount() const;
    [[nodiscard]] uint32_t getLayersCount() const;

private:
    std::unique_ptr<ktxStream> stream;
    std::fstream file;
    std::shared_ptr<ktxTexture2> ktx;
    std::list<Ktx2Chunk> chunks;
};

class ENGINE_API Ktx2FileWriter {
public:
    explicit Ktx2FileWriter(const Path& path, const Vector3i& size, VkFormat format, bool basis = true);
    virtual ~Ktx2FileWriter();

    void writeData(const void* src, bool isNormalMap);

private:
    void writeLevel(const void* src, uint32_t level, const Vector3i& levelSize);
    void compress(bool isNormalMap);
    void writeToFile();

    std::unique_ptr<ktxStream> stream;
    Vector3i size;
    VkFormat format;
    bool basis;
    uint32_t levels;
    std::fstream file;
    std::shared_ptr<ktxTexture2> ktx;
};

ENGINE_API void ktxCompressFile(const Path& src, const Path& dst, bool basis = true);
} // namespace Engine
