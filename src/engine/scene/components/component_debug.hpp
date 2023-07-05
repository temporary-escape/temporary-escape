#pragma once

#include "../../graphics/mesh.hpp"
#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentTransform;
class ENGINE_API ComponentCamera;

class ENGINE_API ComponentDebug : public Component {
public:
    struct Vertex {
        Vector3 position;
        Vector4 color;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                {1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)},
            };
        };
    };

    ComponentDebug() = default;
    explicit ComponentDebug(entt::registry& reg, entt::entity handle);
    virtual ~ComponentDebug() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentDebug);

    void recalculate(VulkanRenderer& vulkan);
    void clear();
    void addBox(const Matrix4& transform, float width, const Color4& color);

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Mesh mesh;
};
} // namespace Engine
