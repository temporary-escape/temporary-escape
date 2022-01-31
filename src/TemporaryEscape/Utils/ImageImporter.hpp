#pragma once

#include "../Graphics/PixelType.hpp"
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "Path.hpp"

namespace Engine {
class ImageImporter {
public:
    virtual ~ImageImporter() = default;

    [[nodiscard]] virtual PixelType getPixelType() const = 0;
    [[nodiscard]] virtual void* getData() const = 0;
    [[nodiscard]] virtual size_t getDataSize() const = 0;
    [[nodiscard]] virtual Vector2i getSize() const = 0;
};
} // namespace Engine
