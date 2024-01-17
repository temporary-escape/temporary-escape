#include "ComponentPointLight.hpp"

using namespace Engine;

ComponentPointLight::ComponentPointLight(EntityId entity, const Color4& color) : Component{entity}, color{color} {
}
