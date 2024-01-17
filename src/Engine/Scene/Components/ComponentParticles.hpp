#pragma once

#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentParticles : public Component {
public:
    ComponentParticles() = default;
    explicit ComponentParticles(EntityId entity);
    COMPONENT_DEFAULTS(ComponentParticles);

    void recalculate(VulkanRenderer& vulkan);
};
} // namespace Engine
