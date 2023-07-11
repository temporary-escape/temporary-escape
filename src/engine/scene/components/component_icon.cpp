#include "component_icon.hpp"
#include "../../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

ComponentIcon::ComponentIcon(entt::registry& reg, entt::entity handle, ImagePtr image, const Vector2& size,
                             const Color4& color) :
    Component{reg, handle}, image{std::move(image)}, size{size}, color{color} {
}

void ComponentIcon::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentIcon>(handle);
}

void ComponentIcon::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentIcon>("ComponentIcon");
    cls["size"] = sol::property(&ComponentIcon::getSize, &ComponentIcon::setSize);
    cls["color"] = sol::property(&ComponentIcon::getColor, &ComponentIcon::setColor);
    cls["image"] = sol::property(&ComponentIcon::getImage, &ComponentIcon::setImage);
    cls["offset"] = sol::property(&ComponentIcon::getOffset, &ComponentIcon::setOffset);
}
