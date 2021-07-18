#include "ComponentPointCloud.hpp"

#include "../Shaders/ShaderPointCloud.hpp"

using namespace Scissio;

ComponentPointCloud::ComponentPointCloud() : mesh{NO_CREATE}, texture{nullptr} {
}

ComponentPointCloud::ComponentPointCloud(Object& object, BasicTexturePtr texture)
    : Component(Type, object), mesh{NO_CREATE}, texture{std::move(texture)} {
}

void ComponentPointCloud::rebuildBuffers() {
    const auto& points = this->getPoints();
    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(points.data(), sizeof(PointCloud::Vertex) * points.size(), VertexBufferUsage::StaticDraw);

    mesh = Mesh{};
    mesh.addVertexBuffer(std::move(vbo), ShaderPointCloud::Position{}, ShaderPointCloud::Size{},
                         ShaderPointCloud::Color{});
    mesh.setPrimitive(PrimitiveType::Points);
    mesh.setCount(static_cast<GLsizei>(points.size()));
}

void ComponentPointCloud::render(ShaderPointCloud& shader) {
    if (!mesh && !this->getPoints().empty()) {
        rebuildBuffers();
    }

    shader.setModelMatrix(getObject().getTransform());
    shader.bindPointTexture(texture->getTexture());
    shader.draw(mesh);
}
