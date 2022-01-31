#include "VertexArray.hpp"
#include <algorithm>

using namespace Engine;

VertexArray::VertexArray(const NoCreate&) : ref(0) {
}

VertexArray::VertexArray() : ref(0) {
    glGenVertexArrays(1, &ref);
}

VertexArray::~VertexArray() {
    if (ref) {
        glDeleteVertexArrays(1, &ref);
    }
}

void VertexArray::bind() const {
    glBindVertexArray(ref);
}

VertexArray::VertexArray(VertexArray&& other) noexcept : ref(0) {
    swap(other);
}

void VertexArray::swap(VertexArray& other) noexcept {
    std::swap(ref, other.ref);
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}
