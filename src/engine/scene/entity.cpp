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

void Entity::setStatic(const bool value) {
    if (!reg) {
        EXCEPTION("Invalid entity");
    }

    if (value && !hasComponent<TagStatic>()) {
        reg->emplace<TagStatic>(handle);
    } else if (hasComponent<TagStatic>()) {
        reg->remove<TagStatic>(handle);
    }
}

void Entity::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Entity>("Entity");
    cls["add_component_transform"] =
        static_cast<ComponentTransform& (Entity::*)()>(&Entity::addComponent<ComponentTransform>);
    cls["add_component_model"] = static_cast<ComponentModel& (Entity::*)()>(&Entity::addComponent<ComponentModel>);
    cls["add_component_rigid_body"] =
        static_cast<ComponentRigidBody& (Entity::*)()>(&Entity::addComponent<ComponentRigidBody>);
}
