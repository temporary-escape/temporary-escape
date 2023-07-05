#include "component_directional_light.hpp"

using namespace Engine;

ComponentDirectionalLight::ComponentDirectionalLight(entt::registry& reg, entt::entity handle, const Color4& color) :
    Component{reg, handle}, color{color} {
}
