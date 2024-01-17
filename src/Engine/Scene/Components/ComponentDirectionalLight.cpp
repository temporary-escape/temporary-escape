#include "ComponentDirectionalLight.hpp"

using namespace Engine;

ComponentDirectionalLight::ComponentDirectionalLight(EntityId entity, const Color4& color) :
    Component{entity}, color{color} {
}
