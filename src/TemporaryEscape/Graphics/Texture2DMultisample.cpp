#include "Texture2DMultisample.hpp"

using namespace Engine;

Texture2DMultisample::Texture2DMultisample(const NoCreate&) : Texture(NO_CREATE) {
}

Texture2DMultisample::Texture2DMultisample() : Texture(GL_TEXTURE_2D_MULTISAMPLE) {
}

Texture2DMultisample::Texture2DMultisample(Texture2DMultisample&& other) noexcept : Texture(std::move(other)) {
}

Texture2DMultisample& Texture2DMultisample::operator=(Texture2DMultisample&& other) noexcept {
    Texture::operator=(std::move(other));
    return *this;
}

void Texture2DMultisample::setStorage(const Vector2i& size, const PixelType pixelType, const int samples) {
    bind();
    const auto internalFormat = toTextureInternalFormat(pixelType);
    glTexImage2DMultisample(target, samples, internalFormat, size.x, size.y, GL_FALSE);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
