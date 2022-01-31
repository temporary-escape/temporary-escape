#include "TextureCubemap.hpp"

using namespace Engine;

TextureCubemap::TextureCubemap(const NoCreate&) : Texture(NO_CREATE) {
}

TextureCubemap::TextureCubemap() : Texture(GL_TEXTURE_CUBE_MAP) {
}

TextureCubemap::TextureCubemap(TextureCubemap&& other) noexcept : Texture(std::move(other)) {
}

TextureCubemap& TextureCubemap::operator=(TextureCubemap&& other) noexcept {
    Texture::operator=(std::move(other));
    return *this;
}

void TextureCubemap::setStorage(const int level, const Vector2i& size, const PixelType pixelType) {
    bind();
    const auto internalFormat = toTextureInternalFormat(pixelType);
    const auto format = toTextureFormat(pixelType);
    const auto type = toTextureType(pixelType);

    for (const auto& side : Sides) {
        glTexImage2D(side, level, internalFormat, size.x, size.y, 0, format, type, nullptr);
    }
}
