#include "FBuffer.hpp"

using namespace Scissio;

void FBuffer::bind(const Vector2i& viewport) {
    if (viewport != this->viewport) {
        this->viewport = viewport;

        fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
        fboColor.setStorage(0, viewport, alpha ? PixelType::Rgba8u : PixelType::Rgb8u);

        fbo.attach(fboColor, FramebufferAttachment::Color0, 0);
        fbo.attach(fboDepth, FramebufferAttachment::DepthStencil, 0);
    } else {
        fbo.bind();
    }

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    glViewport(0, 0, viewport.x, viewport.y);
}

void FBuffer::unbind() {
    Framebuffer::DefaultFramebuffer.bind();
}

void FBuffer::blit(Framebuffer& target) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo.getHandle());
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getHandle());
    glViewport(0, 0, viewport.x, viewport.y);
    glBlitFramebuffer(0, 0, viewport.x, viewport.y, 0, 0, viewport.x, viewport.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
