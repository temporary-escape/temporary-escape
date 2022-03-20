#pragma once

#include "Camera.hpp"
#include "ComponentUserInput.hpp"

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API ComponentCameraTurntable : public Component, public Camera, public UserInputHandler {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentCameraTurntable() = default;
    explicit ComponentCameraTurntable(Object& object) : Component(object), Camera(object) {
    }

    virtual ~ComponentCameraTurntable() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void setZoom(const float value) {
        zoom = value;
    }

    void update();
    void follow(const std::shared_ptr<Entity>& value);
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    Vector3 position{0.0f};
    Vector2 rotation{0.0f};
    bool rotate{false};
    Vector2 mousePosOld;
    float zoom{3.0f};
    std::weak_ptr<Entity> entity;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
