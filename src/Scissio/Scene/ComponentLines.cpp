#include "ComponentLines.hpp"

using namespace Scissio;

ComponentLines::ComponentLines(Object& object) : Component(object), mesh{NO_CREATE} {
}

void ComponentLines::rebuildBuffers() {
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
}
