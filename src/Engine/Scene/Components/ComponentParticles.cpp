#include "ComponentParticles.hpp"

using namespace Engine;

ComponentParticles::ComponentParticles(EntityId entity) : Component{entity} {
}

void ComponentParticles::recalculate(VulkanRenderer& vulkan) {
}
