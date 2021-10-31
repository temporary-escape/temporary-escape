#pragma once

#include "../Graphics/PixelType.hpp"
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "Path.hpp"

#ifndef PNG_H
typedef void* png_structp;
typedef void* png_infop;
#endif

namespace Scissio {
class SCISSIO_API PngImporter {
public:
    explicit PngImporter(const Path& path);
    virtual ~PngImporter();

    int getWidth() const {
        return width;
    }
    int getHeight() const {
        return height;
    }
    int getDepth() const {
        return depth;
    }
    PixelType getPixelType() const {
        return pixelType;
    }
    void* getData() const;
    size_t getDataSize() const {
        return size;
    }
    Vector2i getSize() const {
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
    PixelType pixelType = PixelType::None;
};
} // namespace Scissio
