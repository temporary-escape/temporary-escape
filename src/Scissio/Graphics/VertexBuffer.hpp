#pragma once

#include "../Library.hpp"

#include "Enums.hpp"
#include <glad/glad.h>
#include <string>

namespace Scissio {
struct NoCreate;

class SCISSIO_API VertexBuffer {
public:
    explicit VertexBuffer(const NoCreate&);
    explicit VertexBuffer(VertexBufferType target);
    VertexBuffer(const VertexBuffer& other) = delete;
    VertexBuffer(VertexBuffer&& other) noexcept;
    ~VertexBuffer();

    void swap(VertexBuffer& other) noexcept;
    VertexBuffer& operator=(const VertexBuffer& other) = delete;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void bufferData(const void* data, size_t size, VertexBufferUsage usage);
    void bufferSubData(const void* data, size_t size, size_t offset);
    void bind() const;
    void bindBufferBase(const GLuint index) const;

    GLuint get() const {
        return ref;
    }

    VertexBufferType getType() const {
        return target;
    }

    operator bool() const {
        return ref > 0;
    }

private:
    VertexBufferType target;
    GLuint ref;
};
} // namespace Scissio
