#pragma once

#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentParticles : public Component {
public:
    struct Vertex {
        Vector3 position;
        Vector3 direction;
        Color4 startColor;
        Color4 endColor;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                {2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, direction)},
                {3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, startColor)},
                {4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, endColor)},
            };
        };
    };

    ComponentParticles() = default;
    explicit ComponentParticles(entt::registry& reg, entt::entity handle);
    virtual ~ComponentParticles() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentParticles);

    void recalculate(VulkanRenderer& vulkan);
};
} // namespace Engine
