#include "Texture2D.hpp"

using namespace Scissio;

Texture2D::Texture2D(const NoCreate&) : Texture(NO_CREATE) {
}

Texture2D::Texture2D() : Texture(GL_TEXTURE_2D) {
}

Texture2D::Texture2D(Texture2D&& other) noexcept : Texture(std::move(other)) {
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    Texture::operator=(std::move(other));
    return *this;
}

void Texture2D::setStorage(const int level, const Vector2i& size, const PixelType pixelType) {
    bind();
    const auto internalFormat = toTextureInternalFormat(pixelType);
    const auto format = toTextureFormat(pixelType);
    const auto type = toTextureType(pixelType);
    glTexImage2D(target, level, internalFormat, size.x, size.y, 0, format, type, nullptr);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    // glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1);
}

void Texture2D::setPixels(const int level, const Vector2i& offset, const Vector2i& size, const PixelType pixelType,
                          void* pixels) {

    bind();
    const auto format = toTextureFormat(pixelType);
    const auto type = toTextureType(pixelType);
    glTexSubImage2D(target, level, offset.x, offset.y, size.x, size.y, format, type, pixels);
}
