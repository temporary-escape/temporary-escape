#include "GBuffer.hpp"

using namespace Scissio;

void GBuffer::bind(const Vector2i& viewport) {
    if (viewport != this->viewport) {
        this->viewport = viewport;

        fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
        fboColorRoughness.setStorage(0, viewport, PixelType::Rgba8u);
        fboEmissiveMetallic.setStorage(0, viewport, PixelType::Rgba8u);
        fboNormalAmbient.setStorage(0, viewport, PixelType::Rgba16f);

        fbo.attach(fboColorRoughness, FramebufferAttachment::Color0, 0);
        fbo.attach(fboEmissiveMetallic, FramebufferAttachment::Color1, 0);
        fbo.attach(fboNormalAmbient, FramebufferAttachment::Color2, 0);
        fbo.attach(fboDepth, FramebufferAttachment::DepthStencil, 0);
    } else {
        fbo.bind();
    }

    GLuint attachments[4] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
    };
    glDrawBuffers(4, attachments);

    glViewport(0, 0, viewport.x, viewport.y);
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    glDisable(GL_BLEND);
}

void GBuffer::unbind() {
    Framebuffer::DefaultFramebuffer.bind();

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    glDisable(GL_BLEND);
}
