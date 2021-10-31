#include "ComponentLines.hpp"

#include "../Shaders/ShaderLines.hpp"

using namespace Scissio;

ComponentLines::ComponentLines() : mesh{NO_CREATE} {
}

ComponentLines::ComponentLines(Object& object) : Component(Type, object), mesh{NO_CREATE} {
}

void ComponentLines::rebuildBuffers() {
    const auto& lines = this->getLines();
    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(lines.data(), sizeof(Lines::Vertex) * lines.size(), VertexBufferUsage::StaticDraw);

    mesh = Mesh{};
    mesh.addVertexBuffer(std::move(vbo), ShaderLines::Position{}, ShaderLines::Dummy{}, ShaderLines::Color{});
    mesh.setPrimitive(PrimitiveType::Lines);
    mesh.setCount(static_cast<GLsizei>(lines.size()));
}

void ComponentLines::render(ShaderLines& shader) {
    if (!mesh && !this->getLines().empty()) {
        rebuildBuffers();
    }

    shader.setModelMatrix(getObject().getTransform());
    shader.draw(mesh);
}
