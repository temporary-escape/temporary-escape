#pragma once

#include "../Assets/AssetTexture.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "PointCloud.hpp"

namespace Engine {
class ENGINE_API ComponentPointCloud : public Component, public PointCloud {
public:
    ComponentPointCloud() = default;
    explicit ComponentPointCloud(Object& object, AssetTexturePtr texture)
        : Component(object), mesh{NO_CREATE}, texture{std::move(texture)} {
    }
    virtual ~ComponentPointCloud() = default;

private:
    /*void rebuildBuffers() {
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
    }*/

    Mesh mesh{NO_CREATE};
    AssetTexturePtr texture{nullptr};
};
} // namespace Engine
