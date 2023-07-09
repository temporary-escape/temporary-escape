#include "component_model.hpp"
#include "../../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

ComponentModel::ComponentModel(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

void ComponentModel::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentModel>("ComponentModel");
    cls["model"] = sol::property(&ComponentModel::getModel, &ComponentModel::setModel);
    cls["instanced"] = sol::property(&ComponentModel::isInstanced, &ComponentModel::setInstanced);
}
