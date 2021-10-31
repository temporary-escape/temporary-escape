#pragma once

#include "../Math/Vector.hpp"
#include "PixelType.hpp"
#include "Texture.hpp"

namespace Scissio {
class SCISSIO_API Texture2D : public Texture {
public:
    explicit Texture2D(const NoCreate&);
    explicit Texture2D();
    virtual ~Texture2D() = default;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D(const Texture2D& other) = delete;
    Texture2D& operator=(const Texture2D& other) = delete;
    Texture2D& operator=(Texture2D&& other) noexcept;

    void setStorage(int level, const Vector2i& size, PixelType pixelType);
    void setPixels(int level, const Vector2i& offset, const Vector2i& size, PixelType pixelType, void* pixels);
};
} // namespace Scissio
