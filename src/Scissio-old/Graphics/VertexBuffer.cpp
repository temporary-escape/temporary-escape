#include "VertexBuffer.hpp"
#include <algorithm>

using namespace Scissio;

VertexBuffer::VertexBuffer(const NoCreate&) : ref(0), target(VertexBufferType::Array) {
}

VertexBuffer::VertexBuffer(const VertexBufferType target) : ref(0), target(target) {
    glGenBuffers(1, &ref);
}

VertexBuffer::~VertexBuffer() {
    if (ref) {
        glDeleteBuffers(1, &ref);
    }
}

void VertexBuffer::bufferData(const void* data, const size_t size, VertexBufferUsage usage) {
    bind();
    glBufferData(GLenum(target), size, data, GLenum(usage));
}

void VertexBuffer::bind() const {
    glBindBuffer(GLenum(target), ref);
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept : ref(0) {
    swap(other);
}

void VertexBuffer::swap(VertexBuffer& other) noexcept {
    std::swap(ref, other.ref);
    std::swap(target, other.target);
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}
