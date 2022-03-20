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

    void resize(const Vector2i& viewport) {
        if (size != viewport) {
            fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
            fboColorAlpha.setStorage(0, viewport, PixelType::Rgba8u);
            fboEmissive.setStorage(0, viewport, PixelType::Rgb8u);
            fboMetallicRoughnessAmbient.setStorage(0, viewport, PixelType::Rgb8u);
            fboNormal.setStorage(0, viewport, PixelType::Rgb16f);
            fboObjectId.setStorage(0, viewport, PixelType::Rg8u);

            if (!fboInit) {
                fboInit = true;
                fbo.attach(fboColorAlpha, FramebufferAttachment::Color0, 0);
                fbo.attach(fboEmissive, FramebufferAttachment::Color1, 0);
                fbo.attach(fboMetallicRoughnessAmbient, FramebufferAttachment::Color2, 0);
                fbo.attach(fboNormal, FramebufferAttachment::Color3, 0);
                fbo.attach(fboObjectId, FramebufferAttachment::Color4, 0);
                fbo.attach(fboDepth, FramebufferAttachment::DepthStencil, 0);
            }

            size = viewport;
        }
    }
};

/*struct PostProcessing {
    Vector2i size;
    bool fboInit{false};
    Texture2D fboDepth;

    Framebuffer fbo[2];
    Texture2D fboTexture[2];

    size_t inputIdx{0};
    size_t outputIdx{1};
} postProcessing;*/
} // namespace Engine
