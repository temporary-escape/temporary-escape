#include "ComponentTransform.hpp"
#include "../Entity.hpp"

using namespace Engine;

ComponentTransform::ComponentTransform(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

void ComponentTransform::setParent(const ComponentTransform* value) {
    parent = value;
    if (parent) {
        parentId = static_cast<uint64_t>(parent->getHandle());
    } else {
        parentId = NullParentId;
    }
}

void ComponentTransform::translate(const Vector3& pos) {
    setTransform(glm::translate(transform, pos));
}

void ComponentTransform::move(const Vector3& pos) {
    auto temp = transform;
    temp[3] = Vector4(pos, temp[3].w);
    setTransform(temp);
}

void ComponentTransform::rotate(const Vector3& axis, const float degrees) {
    setTransform(glm::rotate(transform, glm::radians(degrees), axis));
}

void ComponentTransform::rotate(const Quaternion& q) {
    setTransform(transform * glm::toMat4(q));
}

void ComponentTransform::rotateX(const float degrees) {
    rotate(Vector3{1.0f, 0.0f, 0.0f}, degrees);
}

void ComponentTransform::rotateY(const float degrees) {
    rotate(Vector3{0.0f, 1.0f, 0.0f}, degrees);
}

void ComponentTransform::rotateZ(const float degrees) {
    rotate(Vector3{0.0f, 0.0f, 1.0f}, degrees);
}

void ComponentTransform::scale(const Vector3& value) {
    setTransform(glm::scale(transform, value));
}

[[nodiscard]] Matrix4 ComponentTransform::getAbsoluteTransform() const {
    if (const auto p = getParent()) {
        return p->getAbsoluteTransform() * getTransform();
    }
    return getTransform();
}

void ComponentTransform::setTransform(const Matrix4& value) {
    setDirty(true);
    transform = value;
}

Vector3 ComponentTransform::getAbsolutePosition() const {
    if (const auto p = getParent()) {
        return Vector3{getAbsoluteTransform()[3]};
    }
    return getPosition();
}

Quaternion ComponentTransform::getOrientation() const {
    return glm::quat_cast(transform);
}

void ComponentTransform::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentTransform>(handle);
}

void ComponentTransform::setStatic(const bool value) {
    if (value) {
        flags |= static_cast<uint64_t>(TransformFlags::Static);
    } else {
        flags &= ~static_cast<uint64_t>(TransformFlags::Static);
    }
}

bool ComponentTransform::isStatic() const {
    return flags & static_cast<uint64_t>(TransformFlags::Static);
}
