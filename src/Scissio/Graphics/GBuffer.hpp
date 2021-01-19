#pragma once

#include "Framebuffer.hpp"

namespace Scissio {
class SCISSIO_API GBuffer {
public:
    GBuffer() = default;
    virtual ~GBuffer() = default;

    void bind(const Vector2i& viewport);
    void unbind();

    const Texture2D& getDepth() const {
        return fboDepth;
    }

    const Texture2D& getColorRoughness() const {
        return fboColorRoughness;
    }

    const Texture2D& getEmissiveMetallic() const {
        return fboEmissiveMetallic;
    }

    const Texture2D& getNormalAmbient() const {
        return fboNormalAmbient;
    }

private:
    Vector2i viewport;

    Framebuffer fbo;
    Texture2D fboDepth;
    Texture2D fboColorRoughness;
    Texture2D fboEmissiveMetallic;
    Texture2D fboNormalAmbient;
};
} // namespace Scissio
