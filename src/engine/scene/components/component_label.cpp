#include "component_label.hpp"

using namespace Engine;

ComponentLabel::ComponentLabel(entt::registry& reg, entt::entity handle, std::string label) :
    Component{reg, handle}, label{std::move(label)} {
}
