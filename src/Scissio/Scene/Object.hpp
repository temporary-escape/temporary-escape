#pragma once

#include "../Library.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Scissio {
class SCISSIO_API Object {
public:
    explicit Object();
    virtual ~Object() = default;

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

    void updateTransform(const Matrix4& transform) {
        dirty = true;
        this->transform = transform;
    }

    [[nodiscard]] Vector3 getPosition() const {
        return Vector3(transform[3]);
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

private:
    Matrix4 transform;
    bool dirty{true};
};
} // namespace Scissio
