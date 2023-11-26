#include "ComponentPointLight.hpp"

using namespace Engine;

ComponentPointLight::ComponentPointLight(entt::registry& reg, entt::entity handle, const Color4& color) :
    Component{reg, handle}, color{color} {
}
