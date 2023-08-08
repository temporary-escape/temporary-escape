#pragma once

#include "../library.hpp"
#include "../math/vector.hpp"
#include "../utils/path.hpp"
#include "../vulkan/vulkan_types.hpp"

#ifndef PNG_H
typedef void* png_structp;
typedef void* png_infop;
#endif

namespace Engine {
class ENGINE_API PngFileReader {
public:
    explicit PngFileReader(const Path& path);
    virtual ~PngFileReader();

    [[nodiscard]] VkFormat getFormat() const {
        return format;
    }
    [[nodiscard]] const void* getData() const;
    [[nodiscard]] size_t getDataSize() const {
        return size;
    }
    [[nodiscard]] Vector2i getSize() const {
        return {width, height};
    }

private:
    png_structp png;
    png_infop info;
    FILE* file;
    std::string error;
    std::unique_ptr<uint8_t[]> pixels;

    unsigned int img = 0;
    int width = 0;
    int height = 0;
    int depth = 0;
    int bpp = 0;
    size_t size = 0;
    VkFormat format{VkFormat::VK_FORMAT_UNDEFINED};
};
} // namespace Engine
