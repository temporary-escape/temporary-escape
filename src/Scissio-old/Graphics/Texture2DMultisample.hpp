#pragma once

#include "../Math/Vector.hpp"
#include "PixelType.hpp"
#include "Texture.hpp"

namespace Scissio {
class SCISSIO_API Texture2DMultisample : public Texture {
public:
    explicit Texture2DMultisample(const NoCreate&);
    explicit Texture2DMultisample();
    virtual ~Texture2DMultisample() = default;
    Texture2DMultisample(Texture2DMultisample&& other) noexcept;
    Texture2DMultisample(const Texture2DMultisample& other) = delete;
    Texture2DMultisample& operator=(const Texture2DMultisample& other) = delete;
    Texture2DMultisample& operator=(Texture2DMultisample&& other) noexcept;

    void setStorage(const Vector2i& size, PixelType pixelType, int samples);
};
} // namespace Scissio
