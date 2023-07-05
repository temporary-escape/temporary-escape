#include "component_skybox.hpp"

using namespace Engine;

ComponentSkybox::ComponentSkybox(entt::registry& reg, entt::entity handle, uint64_t seed) :
    Component{reg, handle}, seed{seed} {
}
