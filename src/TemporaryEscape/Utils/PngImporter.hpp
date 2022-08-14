#pragma once

#include "ImageImporter.hpp"

#ifndef PNG_H
typedef void* png_structp;
typedef void* png_infop;
#endif

namespace Engine {
class ENGINE_API PngImporter : public ImageImporter {
public:
    explicit PngImporter(const Path& path);
    ~PngImporter() override;

    [[nodiscard]] PixelType getPixelType() const override {
        return pixelType;
    }
    [[nodiscard]] void* getData() const override;
    [[nodiscard]] size_t getDataSize() const override {
        return size;
    }
    [[nodiscard]] Vector2i getSize() const override {
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
    PixelType pixelType = PixelType::Rgba8u;
};
} // namespace Engine
