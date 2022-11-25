#pragma once

#include "../library.hpp"
#include "../math/matrix.hpp"
#include "../math/vector.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API Object {
public:
    explicit Object();
    virtual ~Object() = default;

    void setParentObject(Object* value) {
        parent = value;
    }

    Object* getParentObject() const {
        return parent;
    }

    void translate(const Vector3& pos) {
        updateTransform(glm::translate(transform, pos));
    }

    void move(const Vector3& pos) {
        auto temp = transform;
        temp[3] = Vector4(pos, temp[3].w);
        updateTransform(temp);
    }

    void rotate(const Vector3& axis, const float degrees) {
        updateTransform(glm::rotate(transform, glm::radians(degrees), axis));
    }

    void rotate(const Quaternion& q) {
        updateTransform(transform * glm::toMat4(q));
    }

    void scale(const Vector3& value) {
        updateTransform(glm::scale(transform, value));
    }

    [[nodiscard]] const Matrix4& getTransform() const {
        return transform;
    }

    [[nodiscard]] Matrix4 getAbsoluteTransform() const {
        if (parent) {
            return parent->getAbsoluteTransform() * getTransform();
        }
        return getTransform();
    }

    void updateTransform(const Matrix4& transform) {
        dirty = true;
        this->transform = transform;
    }

    [[nodiscard]] Vector3 getPosition() const {
        return Vector3(transform[3]);
    }

    [[nodiscard]] Vector3 getAbsolutePosition() const {
        if (parent) {
            return parent->getAbsolutePosition() + getPosition();
        }
        return getPosition();
    }

    [[nodiscard]] bool isDirty() const {
        return dirty;
    }

    void markDirty() {
        dirty = true;
    }

    void clearDirty() {
        dirty = false;
    }

    void setVisible(bool value) {
        visible = value;
    }

    bool isVisible() const {
        return visible;
    }

    std::shared_ptr<Entity> asEntity();

private:
    Matrix4 transform;
    bool dirty{true};
    bool visible{true};
    Object* parent{nullptr};

public:
    MSGPACK_DEFINE_ARRAY(transform, visible);
};
} // namespace Engine
