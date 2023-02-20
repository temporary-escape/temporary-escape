#include "component_transform.hpp"
#include "entity.hpp"

using namespace Engine;

void ComponentTransform::setParent(const std::shared_ptr<Entity>& entity) {
    parent = entity;
}

void ComponentTransform::removeParent() {
    parent.reset();
}

void ComponentTransform::translate(const Vector3& pos) {
    updateTransform(glm::translate(transform, pos));
}

void ComponentTransform::move(const Vector3& pos) {
    auto temp = transform;
    temp[3] = Vector4(pos, temp[3].w);
    updateTransform(temp);
}

void ComponentTransform::rotate(const Vector3& axis, const float degrees) {
    updateTransform(glm::rotate(transform, glm::radians(degrees), axis));
}

void ComponentTransform::rotate(const Quaternion& q) {
    updateTransform(transform * glm::toMat4(q));
}

void ComponentTransform::scale(const Vector3& value) {
    updateTransform(glm::scale(transform, value));
}

[[nodiscard]] Matrix4 ComponentTransform::getAbsoluteTransform() const {
    if (const auto p = getParent()) {
        return p->getComponent<ComponentTransform>().getAbsoluteTransform() * getTransform();
    }
    return getTransform();
}

void ComponentTransform::updateTransform(const Matrix4& value) {
    setDirty(true);
    transform = value;
}

Vector3 ComponentTransform::getAbsolutePosition() const {
    if (const auto p = getParent()) {
        return p->getComponent<ComponentTransform>().getAbsolutePosition() + getPosition();
    }
    return getPosition();
}
