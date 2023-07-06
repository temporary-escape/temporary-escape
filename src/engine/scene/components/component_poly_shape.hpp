#pragma once

#include "../../assets/texture.hpp"
#include "../../graphics/mesh.hpp"
#include "../../library.hpp"
#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentPolyShape : public Component {
public:
    struct Point {
        Vector3 position;
        Vector4 color;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Point, position)},
                {1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Point, color)},
            };
        };
    };

    ComponentPolyShape() = default;
    explicit ComponentPolyShape(entt::registry& reg, entt::entity handle);
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