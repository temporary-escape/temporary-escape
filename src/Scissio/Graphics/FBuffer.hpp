#pragma once

#include "Framebuffer.hpp"

namespace Scissio {
class SCISSIO_API FBuffer {
public:
    explicit FBuffer(bool alpha = false) : alpha{alpha} {
    }
    virtual ~FBuffer() = default;

    void bind(const Vector2i& viewport);
    void unbind();
    void blit(Framebuffer& target);

    Framebuffer& getFbo() {
        return fbo;
    }

    const Texture2D& getDepth() const {
        return fboDepth;
    }

    const Texture2D& getColor() const {
        return fboColor;
    }

private:
    bool alpha;

    Vector2i viewport;
    Framebuffer fbo;
    Texture2D fboDepth;
    Texture2D fboColor;
};
} // namespace Scissio
