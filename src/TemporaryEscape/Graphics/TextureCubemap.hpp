#pragma once

#include "../Math/Vector.hpp"
#include "PixelType.hpp"
#include "Texture.hpp"

#include <array>

namespace Engine {
class ENGINE_API TextureCubemap : public Texture {
public:
    static constexpr std::array<GLenum, 6> Sides = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };

    explicit TextureCubemap(const NoCreate&);
    explicit TextureCubemap();
    virtual ~TextureCubemap() = default;
    TextureCubemap(TextureCubemap&& other) noexcept;
    TextureCubemap(const TextureCubemap& other) = delete;
    TextureCubemap& operator=(const TextureCubemap& other) = delete;
    TextureCubemap& operator=(TextureCubemap&& other) noexcept;

    void setStorage(int level, const Vector2i& size, PixelType pixelType);
};
} // namespace Engine
