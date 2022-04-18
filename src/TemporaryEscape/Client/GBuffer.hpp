#pragma once

#include "../Graphics/Framebuffer.hpp"

namespace Engine {
struct GBuffer {
    Vector2i size{};
    Framebuffer fbo;
    bool fboInit{false};
    Texture2D fboDepth;
    Texture2D fboColorAlpha;
    Texture2D fboEmissive;
    Texture2D fboNormal;
    Texture2D fboMetallicRoughnessAmbient;
    Texture2D fboObjectId;

    Texture2D fboFrontColor;
    Framebuffer fboFront;

    Texture2D fboAoResult;
    Framebuffer fboAo;

    Framebuffer fboFxaa;

    Texture2D fboBloomColor;
    Framebuffer fboBloomExtract;

    Texture2D fboBloomBlurResult;
    Framebuffer fboBloomBlurVertical;
    Framebuffer fboBloomBlurHorizontal;
    Framebuffer fboBloomCombine;

    void resize(const Vector2i& viewport) {
        if (size != viewport) {
            fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
            fboColorAlpha.setStorage(0, viewport, PixelType::Rgba8u);
            fboEmissive.setStorage(0, viewport, PixelType::Rgb8u);
            fboMetallicRoughnessAmbient.setStorage(0, viewport, PixelType::Rgb8u);
            fboNormal.setStorage(0, viewport, PixelType::Rgb16f);
            fboObjectId.setStorage(0, viewport, PixelType::Rg8u);

            fboFrontColor.setStorage(0, viewport, PixelType::Rgba8u);

            fboAoResult.setStorage(0, viewport, PixelType::Red8u);

            fboBloomColor.setStorage(0, viewport, PixelType::Rgb8u);
            fboBloomBlurResult.setStorage(0, viewport, PixelType::Rgb8u);

            if (!fboInit) {
                fboInit = true;
                fbo.attach(fboColorAlpha, FramebufferAttachment::Color0, 0);
                fbo.attach(fboEmissive, FramebufferAttachment::Color1, 0);
                fbo.attach(fboMetallicRoughnessAmbient, FramebufferAttachment::Color2, 0);
                fbo.attach(fboNormal, FramebufferAttachment::Color3, 0);
                fbo.attach(fboObjectId, FramebufferAttachment::Color4, 0);
                fbo.attach(fboDepth, FramebufferAttachment::DepthStencil, 0);

                fboFront.attach(fboFrontColor, FramebufferAttachment::Color0, 0);
                fboFront.attach(fboDepth, FramebufferAttachment::DepthStencil, 0);

                fboAo.attach(fboAoResult, FramebufferAttachment::Color0, 0);

                fboFxaa.attach(fboColorAlpha, FramebufferAttachment::Color0, 0);

                fboBloomExtract.attach(fboBloomColor, FramebufferAttachment::Color0, 0);
                fboBloomBlurVertical.attach(fboBloomBlurResult, FramebufferAttachment::Color0, 0);
                fboBloomBlurHorizontal.attach(fboBloomColor, FramebufferAttachment::Color0, 0);
                fboBloomCombine.attach(fboBloomBlurResult, FramebufferAttachment::Color0, 0);
            }

            size = viewport;
        }
    }

    void blit(const Vector2i& viewport, Framebuffer& target) {
        blit(viewport, fboBloomCombine, target, FramebufferAttachment::Color0, BufferBit::Color);
        // blit(viewport, fbo, target, FramebufferAttachment::Color3, BufferBit::Color);
    }

    std::unique_ptr<char[]> readPixels(const Vector2i& viewport) const {
        fboBloomCombine.bind();
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        std::unique_ptr<char[]> pixels(new char[viewport.x * viewport.y * 4]);
        glReadPixels(0, 0, viewport.x, viewport.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
        return pixels;
    }

    void blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target,
              const FramebufferAttachment attachment, BufferBit bufferBit) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, source.getHandle());
        glReadBuffer(GLuint(attachment));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getHandle());
        glViewport(0, 0, viewport.x, viewport.y);
        glBlitFramebuffer(0, 0, viewport.x, viewport.y, 0, 0, viewport.x, viewport.y, GLenum(bufferBit), GL_NEAREST);
    }
};
} // namespace Engine
