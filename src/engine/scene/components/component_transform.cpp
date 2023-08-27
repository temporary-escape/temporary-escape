#include "component_transform.hpp"
#include "../../server/lua.hpp"
#include "../entity.hpp"
#include <sol/sol.hpp>

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
        return p->getAbsolutePosition() + getPosition();
    }
    return getPosition();
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

void ComponentTransform::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentTransform>("ComponentTransform");
    cls["move"] = &ComponentTransform::move;
    cls["translate"] = &ComponentTransform::translate;
    cls["scale"] = &ComponentTransform::scale;
    cls["rotate"] = static_cast<void (ComponentTransform::*)(const Quaternion&)>(&ComponentTransform::rotate);
    cls["rotate_by_axis"] =
        static_cast<void (ComponentTransform::*)(const Vector3&, const float)>(&ComponentTransform::rotate);
    cls["static"] = sol::property(&ComponentTransform::isStatic, &ComponentTransform::setStatic);
    using GetTransformFunc = const Matrix4& (ComponentTransform::*)() const;
    auto GetTransform = static_cast<GetTransformFunc>(&ComponentTransform::getTransform);
    cls["transform"] = sol::property(GetTransform, &ComponentTransform::setTransform);
    cls["parent"] = sol::property(&ComponentTransform::getParent, &ComponentTransform::setParent);
}
