#include "ComponentPointCloud.hpp"

using namespace Scissio;

ComponentPointCloud::ComponentPointCloud() : mesh{NO_CREATE}, texture{nullptr} {
}

ComponentPointCloud::ComponentPointCloud(Object& object, AssetTexturePtr texture)
    : Component(Type, object), mesh{NO_CREATE}, texture{std::move(texture)} {
}

void ComponentPointCloud::rebuildBuffers() {
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, float> Size;
    typedef VertexAttribute<2, Vector4> Color;

    const auto& points = this->getPoints();
    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(points.data(), sizeof(PointCloud::Vertex) * points.size(), VertexBufferUsage::StaticDraw);

    mesh = Mesh{};
    mesh.addVertexBuffer(std::move(vbo), Position{}, Size{}, Color{});
    mesh.setPrimitive(PrimitiveType::Points);
    mesh.setCount(static_cast<GLsizei>(points.size()));
}

void ComponentPointCloud::render(Shader& shader) {
    if (!mesh && !this->getPoints().empty()) {
        rebuildBuffers();
    }

    // shader.setModelMatrix(getObject().getTransform());
    // shader.bindPointTexture(texture->getTexture());
    shader.draw(mesh);
}
