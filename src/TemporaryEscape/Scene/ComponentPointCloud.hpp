#pragma once

#include "../Assets/AssetImage.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "../Shaders/ShaderPointCloud.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentPointCloud : public Component {
public:
    struct Point {
        Vector3 pos{0.0f};
        Vector2 size{64.0f};
        Color4 color{1.0f};
    };

    static_assert(sizeof(Point) == (3 + 2 + 4) * sizeof(float), "size of Point must be tightly packed");

    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentPointCloud() = default;
    explicit ComponentPointCloud(Object& object, AssetTexturePtr texture)
        : Component(object), texture(std::move(texture)) {
    }
    virtual ~ComponentPointCloud() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    const AssetTexturePtr& getTexture() const {
        return texture;
    }

    const Mesh& getMesh() const {
        return mesh;
    }

    void add(const Vector3& pos, const Vector2& size, const Color4& color) {
        setDirty(true);
        points.push_back({pos, size, color});
    }

    void clear() {
        setDirty(true);
        points.clear();
    }

    void recalculate() {
        if (!isDirty()) {
            return;
        }

        if (!vbo) {
            vbo = VertexBuffer(VertexBufferType::Array);
        }

        vbo.bufferData(points.data(), points.size() * sizeof(Point), VertexBufferUsage::DynamicDraw);

        if (!mesh) {
            mesh = Mesh{};
            mesh.addVertexBuffer(vbo, ShaderPointCloud::Position{}, ShaderPointCloud::Size{},
                                 ShaderPointCloud::Color{});
            mesh.setPrimitive(PrimitiveType::Points);
        }

        mesh.setCount(points.size());
        setDirty(false);
    }

private:
    AssetTexturePtr texture;
    std::vector<Point> points;
    Mesh mesh{NO_CREATE};
    VertexBuffer vbo{NO_CREATE};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
