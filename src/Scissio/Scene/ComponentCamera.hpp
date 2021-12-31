#pragma once

#include "Camera.hpp"
#include "Component.hpp"

namespace Scissio {
class SCISSIO_API ComponentCamera : public Component, public Camera {
public:
    ComponentCamera() = default;
    explicit ComponentCamera(Object& object) : Component(object), Camera(object), primary(true) {
    }
    virtual ~ComponentCamera() = default;

    bool getPrimary() const {
        return primary;
    }

    void setPrimary(const bool value) {
        primary = value;
    }

private:
    bool primary{false};

public:
    MSGPACK_DEFINE_ARRAY(primary);
};
} // namespace Scissio
