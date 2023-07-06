#include "component_transform.hpp"
#include "../../server/lua.hpp"
#include "../entity.hpp"
#include <sol/sol.hpp>

using namespace Engine;

ComponentTransform::ComponentTransform(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

void ComponentTransform::setParent(const ComponentTransform& value) {
    parent = &value;
}

void ComponentTransform::removeParent() {
    parent = nullptr;
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
        return p->getAbsoluteTransform() * getTransform();
    }
    return getTransform();
}

void ComponentTransform::updateTransform(const Matrix4& value) {
    setDirty(true);
    transform = value;
}

Vector3 ComponentTransform::getAbsolutePosition() const {
    if (const auto p = getParent()) {
        return p->getAbsolutePosition() + getPosition();
    }
    return getPosition();
}

void ComponentTransform::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentTransform>("ComponentTransform");
    cls["move"] = &ComponentTransform::move;
    cls["translate"] = &ComponentTransform::translate;
}

void ComponentTransform::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentTransform>(handle);
}
