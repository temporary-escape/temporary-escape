#include "component_label.hpp"
#include "../../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

ComponentLabel::ComponentLabel(entt::registry& reg, entt::entity handle, std::string label) :
    Component{reg, handle}, label{std::move(label)} {
}

void ComponentLabel::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentLabel>(handle);
}

void ComponentLabel::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentLabel>("ComponentLabel");
    cls["label"] = sol::property(&ComponentLabel::getLabel, &ComponentLabel::setLabel);
}
