#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "Lines.hpp"

namespace Engine {
class ENGINE_API ComponentLines : public Component, public Lines {
public:
    ComponentLines() = default;
    explicit ComponentLines(Object& object) : Component(object), mesh{NO_CREATE} {
    }
    virtual ~ComponentLines() = default;

private:
    /*void rebuildBuffers() {
        typedef VertexAttribute<0, Vector3> Position;
        typedef VertexAttribute<1, float> Dummy;
        typedef VertexAttribute<2, Color4> Color;

        const auto& lines = this->getLines();
        VertexBuffer vbo(VertexBufferType::Array);
        vbo.bufferData(lines.data(), sizeof(Lines::Vertex) * lines.size(), VertexBufferUsage::StaticDraw);

        mesh = Mesh{};
        mesh.addVertexBuffer(std::move(vbo), Position{}, Dummy{}, Color{});
        mesh.setPrimitive(PrimitiveType::Lines);
        mesh.setCount(static_cast<GLsizei>(lines.size()));
    }*/

    Mesh mesh{NO_CREATE};
};
} // namespace Engine
