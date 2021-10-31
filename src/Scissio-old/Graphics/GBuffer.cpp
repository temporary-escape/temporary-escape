#include "GBuffer.hpp"
#include "FBuffer.hpp"

using namespace Scissio;

void GBuffer::bind(const Vector2i& viewport) {
    if (viewport != this->viewport) {
        this->viewport = viewport;

        fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
        fboColorRoughness.setStorage(0, viewport, PixelType::Rgba8u);
        fboEmissiveMetallic.setStorage(0, viewport, PixelType::Rgba8u);
        fboNormalAmbient.setStorage(0, viewport, PixelType::Rgba16f);
        fboObjectId.setStorage(0, viewport, PixelType::Rg8u);

        fbo.attach(fboColorRoughness, FramebufferAttachment::Color0, 0);
        fbo.attach(fboEmissiveMetallic, FramebufferAttachment::Color1, 0);
        fbo.attach(fboNormalAmbient, FramebufferAttachment::Color2, 0);
        fbo.attach(fboObjectId, FramebufferAttachment::Color3, 0);
        fbo.attach(fboDepth, FramebufferAttachment::DepthStencil, 0);
    } else {
        fbo.bind();
    }

    GLuint attachments[4] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
    };
    glDrawBuffers(4, attachments);

    glViewport(0, 0, viewport.x, viewport.y);
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfv(GL_COLOR, 3, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
}

void GBuffer::unbind() {
    Framebuffer::DefaultFramebuffer.bind();

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);
    // glReadBuffer(GL_COLOR_ATTACHMENT0);
}

uint16_t GBuffer::getObjectIdAtPosition(const Vector2i& pos) {
    glReadBuffer(GL_COLOR_ATTACHMENT3);
    uint8_t pixel[2] = {0};
    glReadPixels(pos.x, viewport.y - pos.y + 1, 1, 1, GL_RG, GL_UNSIGNED_BYTE, pixel);

    return pixel[0] | (pixel[1] << 8);
}

void GBuffer::copyDepth(FBuffer& fBuffer) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo.getHandle());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fBuffer.getFbo().getHandle());
    glViewport(0, 0, viewport.x, viewport.y);
    glBlitFramebuffer(0, 0, viewport.x, viewport.y, 0, 0, viewport.x, viewport.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    fBuffer.getFbo().bind();
}
