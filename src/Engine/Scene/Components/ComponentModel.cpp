#include "ComponentModel.hpp"

using namespace Engine;

ComponentModel::ComponentModel(entt::registry& reg, entt::entity handle, const ModelPtr& model) :
    Component{reg, handle} {
    setModel(model);
}

void ComponentModel::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentModel>(handle);
}
