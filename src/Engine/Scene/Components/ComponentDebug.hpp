#pragma once

#include "../../Graphics/Mesh.hpp"
#include "../Component.hpp"

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
    explicit ComponentDebug(EntityId entity);
    COMPONENT_DEFAULTS(ComponentDebug);

    void recalculate(VulkanRenderer& vulkan);
    void clear();
    void addBox(const Matrix4& transform, float width, const Color4& color);

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

private:
    bool dirty{false};
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Mesh mesh;
};
} // namespace Engine
