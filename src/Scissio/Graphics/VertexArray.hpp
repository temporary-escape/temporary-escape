#pragma once

#include "../Library.hpp"
#include <glad/glad.h>

namespace Scissio {
struct NoCreate;

class SCISSIO_API VertexArray {
public:
    explicit VertexArray(const NoCreate&);
    explicit VertexArray();
    VertexArray(const VertexArray& other) = delete;
    VertexArray(VertexArray&& other) noexcept;
    ~VertexArray();

    void swap(VertexArray& other) noexcept;
    VertexArray& operator=(const VertexArray& other) = delete;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void bind() const;

    GLuint get() const {
        return ref;
    }

    operator bool() const {
        return ref > 0;
    }

private:
    GLuint ref;
};
} // namespace Scissio
