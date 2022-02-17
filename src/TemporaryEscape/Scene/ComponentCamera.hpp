#pragma once

#include "Camera.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentCamera : public Component, public Camera {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentCamera() = default;
    explicit ComponentCamera(Object& object) : Component(object), Camera(object), primary(true) {
    }
    virtual ~ComponentCamera() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    bool getPrimary() const {
        return primary;
    }

    void setPrimary(const bool value) {
        primary = value;
    }

private:
    bool primary{false};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
