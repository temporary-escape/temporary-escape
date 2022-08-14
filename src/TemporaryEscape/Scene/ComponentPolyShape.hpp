#pragma once

#include "../Library.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentPolyShape : public Component {
public:
    struct Point {
        Vector3 pos{0.0f};
        Color4 color{1.0f};
    };

    static_assert(sizeof(Point) == (3 + 4) * sizeof(float), "size of Point must be tightly packed");

    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentPolyShape() = default;
    explicit ComponentPolyShape(Object& object) : Component(object) {
    }
    virtual ~ComponentPolyShape() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    /*const Mesh& getMesh() const {
        return mesh;
    }*/

    void add(const Vector3& pos, const Color4& color) {
        setDirty(true);
        points.push_back({pos, color});
    }

    void clear() {
        setDirty(true);
        points.clear();
    }

    void recalculate() {
        if (!isDirty()) {
            return;
        }

        /*if (!vbo) {
            vbo = VertexBuffer(VertexBufferType::Array);
        }

        vbo.bufferData(points.data(), points.size() * sizeof(Point), VertexBufferUsage::DynamicDraw);

        if (!mesh) {
            mesh = Mesh{};
            mesh.addVertexBuffer(vbo, ShaderPolyShape::Position{}, ShaderPolyShape::Color{});
            mesh.setPrimitive(PrimitiveType::Triangles);
        }

        mesh.setCount(points.size());*/
        setDirty(false);
    }

private:
    std::vector<Point> points;
    // Mesh mesh{NO_CREATE};
    // VertexBuffer vbo{NO_CREATE};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
