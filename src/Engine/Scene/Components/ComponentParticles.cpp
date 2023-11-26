#include "ComponentParticles.hpp"

using namespace Engine;

ComponentParticles::ComponentParticles(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

void ComponentParticles::recalculate(VulkanRenderer& vulkan) {
}
