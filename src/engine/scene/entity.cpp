#include "entity.hpp"
#include "../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

void Entity::reset() {
    reg = nullptr;
}

void Entity::setDisabled(const bool value) {
    if (!reg) {
        EXCEPTION("Invalid entity");
    }

    if (value && !hasComponent<TagDisabled>()) {
        reg->emplace<TagDisabled>(handle);
    } else if (hasComponent<TagDisabled>()) {
        reg->remove<TagDisabled>(handle);
    }
}

bool Entity::isDisabled() const {
    return reg && reg->try_get<TagDisabled>(handle);
}

void Entity::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Entity>("Entity");
    cls["add_component_transform"] =
        static_cast<ComponentTransform& (Entity::*)()>(&Entity::addComponent<ComponentTransform>);
    cls["add_component_model"] =
        static_cast<ComponentModel& (Entity::*)(const ModelPtr&)>(&Entity::addComponent<ComponentModel>);
    cls["add_component_rigid_body"] =
        static_cast<ComponentRigidBody& (Entity::*)()>(&Entity::addComponent<ComponentRigidBody>);
    cls["add_component_icon"] =
        static_cast<ComponentIcon& (Entity::*)(const ImagePtr&)>(&Entity::addComponent<ComponentIcon>);
    cls["add_component_label"] =
        static_cast<ComponentLabel& (Entity::*)(const std::string&)>(&Entity::addComponent<ComponentLabel>);
    cls["add_component_script"] = [l = &lua](Entity& self, const sol::table& instance) -> ComponentScript& {
        return self.addComponent<ComponentScript>(instance);
    };

    cls["has_component_transform"] = &Entity::hasComponent<ComponentTransform>;
    cls["has_component_model"] = &Entity::hasComponent<ComponentModel>;
    cls["has_component_rigid_body"] = &Entity::hasComponent<ComponentRigidBody>;
    cls["has_component_icon"] = &Entity::hasComponent<ComponentIcon>;
    cls["has_component_label"] = &Entity::hasComponent<ComponentLabel>;
    cls["has_component_script"] = &Entity::hasComponent<ComponentScript>;

    cls["get_component_transform"] = &Entity::tryGetComponent<ComponentTransform>;
    cls["get_component_model"] = &Entity::tryGetComponent<ComponentModel>;
    cls["get_component_rigid_body"] = &Entity::tryGetComponent<ComponentRigidBody>;
    cls["get_component_icon"] = &Entity::tryGetComponent<ComponentIcon>;
    cls["get_component_label"] = &Entity::tryGetComponent<ComponentLabel>;
    cls["get_component_script"] = &Entity::tryGetComponent<ComponentScript>;

    cls["script"] = [](Entity& self) -> sol::table {
        auto* component = self.tryGetComponent<ComponentScript>();
        if (component) {
            return component->getInstance();
        }
        return nullptr;
    };
}
