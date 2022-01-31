#include "Mesh.hpp"

#include "Texture.hpp"

using namespace Engine;

Mesh::Mesh(const NoCreate&) : vao{NO_CREATE}, ibo{NO_CREATE} {
}

Mesh::Mesh() : vao{}, ibo{NO_CREATE} {
}

Mesh::Mesh(Mesh&& other) noexcept : vao{NO_CREATE}, ibo{NO_CREATE} {
    swap(other);
}

void Mesh::swap(Mesh& other) noexcept {
    std::swap(vao, other.vao);
    std::swap(ibo, other.ibo);
    std::swap(vbos, other.vbos);
    std::swap(indexType, other.indexType);
    std::swap(primitiveType, other.primitiveType);
    std::swap(count, other.count);
    std::swap(instances, other.instances);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Mesh::setIndexBuffer(VertexBuffer&& ibo, const IndexType indexType) {
    this->ibo = std::move(ibo);
    bind();
    this->ibo.bind();
    setIndexType(indexType);
}

void Mesh::setIndexBuffer(const VertexBuffer& ibo, const IndexType indexType) {
    bind();
    ibo.bind();
    setIndexType(indexType);
}

void Mesh::setIndexType(const IndexType indexType) {
    this->indexType = indexType;
}

void Mesh::setCount(const GLsizei count) {
    this->count = count;
}

void Mesh::setPrimitive(const PrimitiveType primitiveType) {
    this->primitiveType = primitiveType;
}

void Mesh::bind() const {
    vao.bind();
}
