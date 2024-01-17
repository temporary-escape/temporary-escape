#pragma once

#include "../../Assets/Texture.hpp"
#include "../../Graphics/Mesh.hpp"
#include "../../Library.hpp"
#include "../Component.hpp"

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
    explicit ComponentPolyShape(EntityId entity);
    COMPONENT_DEFAULTS(ComponentPolyShape);

    void recalculate(VulkanRenderer& vulkan);

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

    void add(const Vector3& pos, const Color4& color) {
        dirty = true;
        points.push_back({pos, color});
    }

    void clear() {
        dirty = true;
        points.clear();
    }

private:
    bool dirty{false};
    std::vector<Point> points;
    Mesh mesh;
};
} // namespace Engine
