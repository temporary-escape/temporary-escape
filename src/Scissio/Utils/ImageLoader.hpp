#pragma once

#include "../Graphics/PixelType.hpp"
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "Path.hpp"

#include <mutex>

namespace Scissio {
class SCISSIO_API ImageLoader {
public:
    explicit ImageLoader(const Path& path);
    virtual ~ImageLoader();

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
    static std::mutex mutex;

    unsigned int img = 0;
    int width = 0;
    int height = 0;
    int depth = 0;
    size_t size = 0;
    PixelType pixelType = PixelType::None;
};
} // namespace Scissio
