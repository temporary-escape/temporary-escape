#pragma once

#include "../library.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPolyShape : public Component {
public:
    struct Point {
        Vector3 pos{0.0f};
        Color4 color{1.0f};
    };

    static_assert(sizeof(Point) == (3 + 4) * sizeof(float), "size of Point must be tightly packed");

    ComponentPolyShape() = default;
    virtual ~ComponentPolyShape() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPolyShape);

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

private:
    std::vector<Point> points;
    // Mesh mesh{NO_CREATE};
    // VertexBuffer vbo{NO_CREATE};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
