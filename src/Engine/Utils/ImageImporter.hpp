#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "Path.hpp"

namespace Engine {
class ImageImporter {
public:
    enum class PixelType {
        Rgb8u,
        Rgba8u,
        Rgb16u,
        Rgba16u,
    };

    virtual ~ImageImporter() = default;

    [[nodiscard]] virtual PixelType getPixelType() const = 0;
    [[nodiscard]] virtual void* getData() const = 0;
    [[nodiscard]] virtual size_t getDataSize() const = 0;
    [[nodiscard]] virtual Vector2i getSize() const = 0;
};
} // namespace Engine
