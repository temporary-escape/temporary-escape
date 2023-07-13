#include "component_model.hpp"
#include "../../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

ComponentModel::ComponentModel(entt::registry& reg, entt::entity handle, const ModelPtr& model) :
    Component{reg, handle} {
    setModel(model);
}

void ComponentModel::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentModel>(handle);
}

void ComponentModel::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentModel>("ComponentModel");
    cls["model"] = sol::property(&ComponentModel::getModel, &ComponentModel::setModel);
}
