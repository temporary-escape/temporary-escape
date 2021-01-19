#pragma once

#include "../Library.hpp"
#include "PixelType.hpp"
#include "Renderbuffer.hpp"
#include "Texture2D.hpp"
#include "Texture2DMultisample.hpp"

namespace Scissio {
struct NoCreate;

class SCISSIO_API Framebuffer {
public:
    explicit Framebuffer(const NoCreate&);
    explicit Framebuffer();
    Framebuffer(const Framebuffer& other) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    virtual ~Framebuffer();
    void swap(Framebuffer& other) noexcept;
    Framebuffer& operator=(const Framebuffer& other) = delete;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void attach(const Texture2D& texture, FramebufferAttachment attachment, int level);
    void attach(const Texture2DMultisample& texture, FramebufferAttachment attachment);
    void attach(const Renderbuffer& renderbuffer, FramebufferAttachment attachment);
    void bind() const;

    GLuint getHandle() const {
        return handle;
    }

    static Framebuffer DefaultFramebuffer;

private:
    GLuint handle;
};
} // namespace Scissio
