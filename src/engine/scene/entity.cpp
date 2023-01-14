#include "entity.hpp"

using namespace Engine;

void Entity::destroy() {
    if (reg) {
        reg->destroy(handle);
    }
}

void Entity::setDisabled(bool value) {
    if (disabled != value) {
        if (!disabled) {
            reg->emplace<TagDisabled>(handle);
        } else {
            reg->remove<TagDisabled>(handle);
        }
    }
    disabled = value;
}
