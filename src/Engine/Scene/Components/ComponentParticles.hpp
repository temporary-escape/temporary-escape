#pragma once

#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentParticles : public Component {
public:
    struct ParticlesBatchUniform {
        Matrix4 modelMatrix;
        alignas(4) float timeDelta;
        alignas(4) float strength;
        alignas(4) float alpha;
    };

    ComponentParticles() = default;
    explicit ComponentParticles(EntityId entity);
    COMPONENT_DEFAULTS(ComponentParticles);

    void recalculate(VulkanRenderer& vulkan);
};
} // namespace Engine
