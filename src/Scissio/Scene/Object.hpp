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
        transform = glm::translate(transform, pos);
    }

    void move(const Vector3& pos) {
        transform[3] = Vector4(pos, transform[3].w);
    }

    void rotate(const Vector3& axis, const float degrees) {
        transform = glm::rotate(transform, glm::radians(degrees), axis);
    }

    void rotate(const Quaternion& q) {
        transform = transform * glm::toMat4(q);
    }

    const Matrix4& getTransform() const {
        return transform;
    }

    Matrix4& getTransform() {
        return transform;
    }

    Vector3 getPosition() const {
        return Vector3(transform[3]);
    }

private:
    Matrix4 transform;
};
} // namespace Scissio
