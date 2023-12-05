#include "ComponentLabel.hpp"

using namespace Engine;

ComponentLabel::ComponentLabel(entt::registry& reg, entt::entity handle, std::string label) :
    Component{reg, handle}, label{std::move(label)} {
}

void ComponentLabel::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentLabel>(handle);
}
