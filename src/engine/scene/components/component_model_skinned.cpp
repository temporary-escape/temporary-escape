#include "component_model_skinned.hpp"
#include "../../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

ComponentModelSkinned::ComponentModelSkinned(entt::registry& reg, entt::entity handle, const ModelPtr& model) :
    Component{reg, handle} {
    setModel(model);
}

void ComponentModelSkinned::setModel(ModelPtr value) {
    model = std::move(value);
}

void ComponentModelSkinned::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentModelSkinned>(handle);
}

void ComponentModelSkinned::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentModelSkinned>("ComponentModelSkinned");
    cls["model"] = sol::property(&ComponentModelSkinned::getModel, &ComponentModelSkinned::setModel);
}
