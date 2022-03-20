#pragma once

#include "../Math/Vector.hpp"
#include "PixelType.hpp"
#include "Texture.hpp"

#include <array>

namespace Engine {
class ENGINE_API TextureCubemap : public Texture {
public:
    static constexpr std::array<CubemapSide, 6> sides = {
        CubemapSide::PositiveX, CubemapSide::NegativeX, CubemapSide::PositiveY,
        CubemapSide::NegativeY, CubemapSide::PositiveZ, CubemapSide::NegativeZ,
    };

    explicit TextureCubemap(const NoCreate&);
    explicit TextureCubemap();
    virtual ~TextureCubemap() = default;
    TextureCubemap(TextureCubemap&& other) noexcept;
    TextureCubemap(const TextureCubemap& other) = delete;
    TextureCubemap& operator=(const TextureCubemap& other) = delete;
    TextureCubemap& operator=(TextureCubemap&& other) noexcept;

    void setStorage(int level, const Vector2i& size, PixelType pixelType);
    void setPixels(int level, const Vector2i& offset, CubemapSide side, const Vector2i& size, PixelType pixelType,
                   const void* pixels);
};
} // namespace Engine
