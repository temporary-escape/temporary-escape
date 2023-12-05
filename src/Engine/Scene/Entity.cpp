#include "Entity.hpp"

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
