#include "Renderbuffer.hpp"

#include <memory>

using namespace Scissio;

Renderbuffer::Renderbuffer(const NoCreate&) : handle(0) {
}

Renderbuffer::Renderbuffer() : handle(0) {
    glGenRenderbuffers(1, &handle);
}

Renderbuffer::~Renderbuffer() {
    if (handle > 0) {
        glDeleteRenderbuffers(1, &handle);
    }
}

Renderbuffer::Renderbuffer(Renderbuffer&& other) noexcept : handle(0) {
    swap(other);
}

void Renderbuffer::swap(Renderbuffer& other) noexcept {
    std::swap(handle, other.handle);
}

Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Renderbuffer::setStorage(const Vector2i& size, PixelType pixelType) {
    const auto internalFormat = toTextureInternalFormat(pixelType);
    bind();
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, size.x, size.y);
}

void Renderbuffer::bind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, handle);
}
