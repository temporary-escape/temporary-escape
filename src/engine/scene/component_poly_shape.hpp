#pragma once

#include "../assets/texture.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/shaders/shader_component_poly_shape.hpp"
#include "../library.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPolyShape : public Component {
public:
    using Point = ShaderComponentPolyShape::Vertex;

    ComponentPolyShape() = default;
    virtual ~ComponentPolyShape() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPolyShape);

    void recalculate(VulkanRenderer& vulkan);

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

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
    Mesh mesh;
};
} // namespace Engine
