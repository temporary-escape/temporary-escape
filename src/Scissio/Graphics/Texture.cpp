#include "Texture.hpp"

#include <memory>

using namespace Scissio;

Texture::Texture(const NoCreate&) : handle(0), target(0) {
}

Texture::Texture(const GLuint target) : handle(0), target(target) {
    glGenTextures(1, &handle);
}

Texture::~Texture() {
    if (handle != 0) {
        glDeleteTextures(1, &handle);
    }
}

Texture::Texture(Texture&& other) noexcept : handle(0), target(0) {
    swap(other);
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Texture::swap(Texture& other) noexcept {
    std::swap(handle, other.handle);
    std::swap(target, other.target);
}

void Texture::bind() const {
    bind(0);
}

void Texture::bind(const GLuint location) const {
    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(target, handle);
}

void Texture::unbind() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, 0);
}

void Texture::generateMipmaps() {
    bind();
    glGenerateMipmap(target);
}

void Texture::texParameteri(const GLenum parameter, const GLint value) {
    glTexParameteri(target, parameter, value);
}

void Texture::setFiltering(TextureFiltering min, TextureFiltering mag) {
    bind();
    texParameteri(GL_TEXTURE_MIN_FILTER, GLenum(min));
    texParameteri(GL_TEXTURE_MAG_FILTER, GLenum(mag));
}

void Texture::setWrapping(TextureWrapping s, TextureWrapping t) {
    bind();
    texParameteri(GL_TEXTURE_WRAP_S, GLenum(s));
    texParameteri(GL_TEXTURE_WRAP_T, GLenum(t));
}
