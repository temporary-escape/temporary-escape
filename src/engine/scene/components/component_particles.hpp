#pragma once

#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentParticles : public Component {
public:
    ComponentParticles() = default;
    explicit ComponentParticles(entt::registry& reg, entt::entity handle);
    virtual ~ComponentParticles() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentParticles);

    void recalculate(VulkanRenderer& vulkan);
};
} // namespace Engine
