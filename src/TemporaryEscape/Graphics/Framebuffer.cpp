#include "Framebuffer.hpp"

#include <memory>

using namespace Engine;

Framebuffer::Framebuffer(const NoCreate&) : handle(0) {
}

Framebuffer::Framebuffer() : handle(0) {
    glGenFramebuffers(1, &handle);
}

Framebuffer::~Framebuffer() {
    if (handle > 0) {
        glDeleteFramebuffers(1, &handle);
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept : handle(0) {
    swap(other);
}

void Framebuffer::swap(Framebuffer& other) noexcept {
    std::swap(handle, other.handle);
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Framebuffer::attach(const Texture2D& texture, const FramebufferAttachment attachment, const int level) {
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GLenum(attachment), texture.getTarget(), texture.getHandle(), level);
}

void Framebuffer::attach(const Texture2DMultisample& texture, const FramebufferAttachment attachment) {
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GLenum(attachment), texture.getTarget(), texture.getHandle(), 0);
}

void Framebuffer::attach(const Renderbuffer& renderbuffer, const FramebufferAttachment attachment) {
    bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GLenum(attachment), GL_RENDER, renderbuffer.getHandle());
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

Framebuffer Framebuffer::DefaultFramebuffer{NO_CREATE};
