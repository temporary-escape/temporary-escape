#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "PixelType.hpp"

namespace Scissio {
struct NoCreate;

class SCISSIO_API Renderbuffer {
public:
    explicit Renderbuffer(const NoCreate&);
    explicit Renderbuffer();
    Renderbuffer(const Renderbuffer& other) = delete;
    Renderbuffer(Renderbuffer&& other) noexcept;
    virtual ~Renderbuffer();
    void swap(Renderbuffer& other) noexcept;
    Renderbuffer& operator=(const Renderbuffer& other) = delete;
    Renderbuffer& operator=(Renderbuffer&& other) noexcept;

    void setStorage(const Vector2i& size, PixelType pixelType);
    void bind() const;

    GLuint getHandle() const {
        return handle;
    }

private:
    GLuint handle;
};
} // namespace Scissio