#include "component_icon.hpp"
#include "../../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

ComponentIcon::ComponentIcon(entt::registry& reg, entt::entity handle, ImagePtr image) :
    Component{reg, handle}, image{std::move(image)} {
}

void ComponentIcon::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentIcon>(handle);
}

void ComponentIcon::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentIcon>("ComponentIcon");
    cls["image"] = sol::property(&ComponentIcon::getImage, &ComponentIcon::setImage);
    cls["offset"] = sol::property(&ComponentIcon::getOffset, &ComponentIcon::setOffset);
    cls["selectable"] = sol::property(&ComponentIcon::isSelectable, &ComponentIcon::setSelectable);
}
