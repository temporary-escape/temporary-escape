#include "entity.hpp"

using namespace Engine;

Entity::~Entity() {
    for (auto& child : children) {
        child->setParentObject(nullptr);
    }
    if (auto parent = getParentObject()) {
        dynamic_cast<Entity*>(parent)->removeChildInternal(this);
    }
}

Entity::Delta Entity::getDelta() const {
    Delta delta{};
    delta.id = id;
    delta.transform = getTransform();
    delta.visible = isVisible();

    for (const auto& ref : components) {
        if (ref.ptr->isDirty()) {
            delta.components.emplace_back();
            EntityComponentHelper::getDelta(ref.ptr, ref.type, delta.components.back());
            ref.ptr->setDirty(false);
        }
    }
    return delta;
}

void Entity::applyDelta(const Delta& delta) {
    updateTransform(delta.transform);
    setVisible(delta.visible);
}

void Entity::setParent(const std::shared_ptr<Entity>& value) {
    if (value) {
        setParentObject(value.get());
        value->addChild(shared_from_this());
    } else {
        if (const auto ptr = getParentObject()) {
            dynamic_cast<Entity*>(ptr)->removeChildInternal(this);
        }
        setParentObject(nullptr);
    }
}

void Entity::removeChild(const std::shared_ptr<Entity>& value) {
    const auto it = children.erase(std::remove(children.begin(), children.end(), value), children.end());
    if (it != children.end()) {
        value->setParent(nullptr);
    }
}

void Entity::clearChildren() {
    children.clear();
}
