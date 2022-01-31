#pragma once

#include "Camera.hpp"
#include "ComponentUserInput.hpp"

namespace Engine {
class ENGINE_API ComponentCameraTurntable : public Component, public Camera, public UserInputHandler {
public:
    ComponentCameraTurntable() = default;
    explicit ComponentCameraTurntable(Object& object) : Component(object), Camera(object) {
    }

    virtual ~ComponentCameraTurntable() = default;

    void update() {
        glm::mat4x4 transform{1.0f};
        transform = glm::rotate(transform, glm::radians(rotation.x), Vector3{0.0f, 1.0f, 0.0f});
        transform = glm::rotate(transform, glm::radians(rotation.y), Vector3{1.0f, 0.0f, 0.0f});

        const auto eyes = position + Vector3(transform * Vector4(0.0f, 0.0f, zoom, 1.0f));
        lookAt(eyes, position);
    }

    void eventMouseMoved(const Vector2i& pos) override {
        if (rotate) {
            rotation += (mousePosOld - Vector2(pos)) * 0.2f;
            while (rotation.x > 360.0f) {
                rotation.x -= 360.0f;
            }
            while (rotation.x < 0.0f) {
                rotation.x += 360.0f;
            }
            rotation.y = glm::clamp(rotation.y, -89.0f, 89.0f);
            mousePosOld = pos;
        }
    }

    void eventMousePressed(const Vector2i& pos, const MouseButton button) override {
        mousePosOld = pos;
        rotate |= button == MouseButton::Right;
    }

    void eventMouseReleased(const Vector2i& pos, const MouseButton button) override {
        mousePosOld = pos;
        rotate &= !(button == MouseButton::Right);
    }

    void eventMouseScroll(const int xscroll, const int yscroll) override {
        const auto factor = map(zoom, 1.0f, 500.0f, 0.2f, 20.0f);
        zoom += static_cast<float>(-yscroll) * factor;

        if (zoom < 1.0f) {
            zoom = 1.0f;
        } else if (zoom > 500.0f) {
            zoom = 500.0f;
        }
    }

    void eventKeyPressed(const Key key, const Modifiers modifiers) override {
    }

    void eventKeyReleased(const Key key, const Modifiers modifiers) override {
    }

    bool getPrimary() const {
        return primary;
    }

    void setPrimary(const bool value) {
        primary = value;
    }

private:
    Vector3 position{0.0f};
    bool primary{true};
    Vector2 rotation{0.0f};
    bool rotate{false};
    Vector2 mousePosOld;
    float zoom{3.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
