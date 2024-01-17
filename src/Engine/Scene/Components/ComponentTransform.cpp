#include "ComponentTransform.hpp"
#include "../Entity.hpp"

using namespace Engine;

ComponentTransform::ComponentTransform(EntityId entity) : Component{entity} {
}

void ComponentTransform::setParent(const ComponentTransform* value) {
    parent = value;
    if (parent) {
        parentId = static_cast<uint64_t>(parent->getEntity());
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

Matrix4 ComponentTransform::getAbsoluteTransform() const {
    if (const auto p = getParent()) {
        return p->getAbsoluteTransform() * getTransform();
    }
    return getTransform();
}

Matrix4 ComponentTransform::getAbsoluteInterpolatedTransform() const {
    if (const auto p = getParent()) {
        return p->getAbsoluteInterpolatedTransform() * getInterpolatedTransform();
    }
    return getInterpolatedTransform();
}

void ComponentTransform::setTransform(const Matrix4& value) {
    transform = value;
}

Vector3 ComponentTransform::getAbsolutePosition() const {
    if (const auto p = getParent()) {
        return Vector3{getAbsoluteTransform()[3]};
    }
    return getPosition();
}

Vector3 ComponentTransform::getAbsoluteInterpolatedPosition() const {
    if (const auto p = getParent()) {
        return Vector3{getAbsoluteInterpolatedTransform()[3]};
    }
    return getInterpolatedPosition();
}

Quaternion ComponentTransform::getOrientation() const {
    return glm::quat_cast(transform);
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

void ComponentTransform::interpolate() {
    if (glm::distance2(transformInterpolated[3], transform[3]) > 50 * 50) {
        transformInterpolated = transform;
    } else {
        const auto previousPos = transformInterpolated[3];
        const auto previousRot = glm::quat_cast(transformInterpolated);

        const auto rotDiff = glm::slerp(previousRot, glm::quat_cast(transform), 0.5f);

        transformInterpolated = glm::toMat4(rotDiff);
        transformInterpolated[3] = glm::mix(previousPos, transform[3], 0.5f);
        transformInterpolated[3].w = 1.0f;
    }
    interpolated = true;
}
