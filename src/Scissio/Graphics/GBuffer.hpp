#pragma once

#include "Framebuffer.hpp"

namespace Scissio {
class FBuffer;

class SCISSIO_API GBuffer {
public:
    GBuffer() = default;
    virtual ~GBuffer() = default;

    void bind(const Vector2i& viewport);
    void unbind();
    void copyDepth(FBuffer& fBuffer);
    uint16_t getObjectIdAtPosition(const Vector2i& pos);

    Framebuffer& getFbo() {
        return fbo;
    }

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

    const Texture2D& getObjectId() const {
        return fboObjectId;
    }

private:
    Vector2i viewport;

    Framebuffer fbo;
    Texture2D fboDepth;
    Texture2D fboColorRoughness;
    Texture2D fboEmissiveMetallic;
    Texture2D fboNormalAmbient;
    Texture2D fboObjectId;
};
} // namespace Scissio
