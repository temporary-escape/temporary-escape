#pragma once

#include "Texture.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace Scissio {
template <GLint I, typename T> struct VertexAttribute {
    static constexpr GLint index = I;
    static constexpr GLsizei size = sizeof(T);
    static constexpr GLint components = 1;

    static void bindAttribute(const GLsizei stride, const GLsizei offset) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, components, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset)));
    }

    static void bindDivisor() {
        glVertexAttribDivisor(index, 1);
    }
};

template <GLint I, GLint L, typename T> struct VertexAttribute<I, glm::vec<L, T>> {
    static constexpr GLint index = I;
    static constexpr GLsizei size = sizeof(glm::vec<L, T>);
    static constexpr GLint components = L;

    static void bindAttribute(const GLsizei stride, const GLsizei offset) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, components, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset)));
    }

    static void bindDivisor() {
        glVertexAttribDivisor(index, 1);
    }
};

template <GLint I, typename T> struct VertexAttribute<I, glm::mat<4, 4, T>> {
    static constexpr GLint index = I;
    static constexpr GLsizei size = sizeof(glm::mat<4, 4, T>);
    static constexpr GLint components = 16;

    static void bindAttribute(const GLsizei stride, const GLsizei offset) {
        static constexpr auto vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(index + 0);
        glVertexAttribPointer(index + 0, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset + vec4Size * 0)));
        glEnableVertexAttribArray(index + 1);
        glVertexAttribPointer(index + 1, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset + vec4Size * 1)));
        glEnableVertexAttribArray(index + 2);
        glVertexAttribPointer(index + 2, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset + vec4Size * 2)));
        glEnableVertexAttribArray(index + 3);
        glVertexAttribPointer(index + 3, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset + vec4Size * 3)));
    }

    static void bindDivisor() {
        glVertexAttribDivisor(index + 0, 1);
        glVertexAttribDivisor(index + 1, 1);
        glVertexAttribDivisor(index + 2, 1);
        glVertexAttribDivisor(index + 3, 1);
    }
};

class Mesh {
public:
    explicit Mesh(const NoCreate&);
    explicit Mesh();
    Mesh(const Mesh& other) = delete;
    Mesh(Mesh&& other) noexcept;
    virtual ~Mesh() = default;
    void swap(Mesh& other) noexcept;
    Mesh& operator=(const Mesh& other) = delete;
    Mesh& operator=(Mesh&& other) noexcept;

    void setIndexBuffer(VertexBuffer&& ibo, IndexType indexType);
    void setIndexBuffer(const VertexBuffer& ibo, IndexType indexType);

    template <typename... Attributes> void addVertexBuffer(VertexBuffer&& vbo, Attributes&&... attributes) {
        bind();
        vbos.push_back(std::move(vbo));
        vbos.back().bind();

        const auto total = getTotal(0, attributes...);
        setAttributes(0, total, attributes...);
    }

    template <typename... Attributes> void addVertexBufferInstanced(VertexBuffer&& vbo, Attributes&&... attributes) {
        addVertexBuffer(std::move(vbo), attributes...);
        setDivisor(attributes...);
    }

    template <typename... Attributes> void addVertexBuffer(const VertexBuffer& vbo, Attributes&&... attributes) {
        bind();
        vbo.bind();

        const auto total = getTotal(0, attributes...);
        setAttributes(0, total, attributes...);
    }

    template <typename... Attributes>
    void addVertexBufferInstanced(const VertexBuffer& vbo, Attributes&&... attributes) {
        addVertexBuffer(vbo, attributes...);
        setDivisor(attributes...);
    }

    void setIndexType(IndexType indexType);
    void setCount(GLsizei count);
    void setPrimitive(PrimitiveType primitiveType);
    void bind() const;

    PrimitiveType getPrimitive() const {
        return primitiveType;
    }

    IndexType getIndexType() const {
        return indexType;
    }

    GLsizei getCount() const {
        return count;
    }

    GLsizei getInstancesCount() const {
        return instances;
    }

    void setInstancesCount(const GLsizei instances) {
        this->instances = instances;
    }

    bool hasIbo() const {
        return indexType != IndexType::None;
    }

    /*const VertexBuffer& getIbo() const {
        return *iboTarget;
    }*/

private:
    template <typename Attribute> static GLsizei getTotal(const GLsizei total, const Attribute& attribute) {
        return total + attribute.size;
    }

    template <typename Attribute, typename... Attributes>
    static GLsizei getTotal(const GLsizei total, const Attribute& attribute, Attributes&&... attributes) {
        return getTotal(total + attribute.size, attributes...);
    }

    template <typename Attribute>
    static void setAttributes(const GLsizei offset, const GLsizei stride, const Attribute& attribute) {
        /*glEnableVertexAttribArray(attribute.index);
        glVertexAttribPointer(attribute.index, attribute.components, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset)));*/
        Attribute::bindAttribute(stride, offset);
    }

    template <typename Attribute, typename... Attributes>
    static void setAttributes(const GLsizei offset, const GLsizei stride, const Attribute& attribute,
                              Attributes&&... attributes) {
        /*glEnableVertexAttribArray(attribute.index);
        glVertexAttribPointer(attribute.index, attribute.components, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<size_t>(offset)));*/
        Attribute::bindAttribute(stride, offset);

        setAttributes(offset + attribute.size, stride, attributes...);
    }

    template <typename Attribute> static void setDivisor(const Attribute& attribute) {
        Attribute::bindDivisor();
    }

    template <typename Attribute, typename... Attributes>
    static void setDivisor(const Attribute& attribute, Attributes&&... attributes) {
        Attribute::bindDivisor();

        setDivisor(attributes...);
    }

    VertexArray vao;
    VertexBuffer ibo;
    std::vector<VertexBuffer> vbos{};
    PrimitiveType primitiveType{PrimitiveType::Triangles};
    IndexType indexType{IndexType::None};
    GLsizei count{0};
    GLsizei instances{0};
};
} // namespace Scissio
