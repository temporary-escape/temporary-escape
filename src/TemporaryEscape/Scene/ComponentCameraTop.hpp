#pragma once

#include "Camera.hpp"
#include "ComponentUserInput.hpp"

namespace Engine {
class ENGINE_API ComponentCameraTop : public Component, public Camera, public UserInputHandler {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentCameraTop() = default;
    explicit ComponentCameraTop(Object& object) : Component(object), Camera(object) {
    }

    virtual ~ComponentCameraTop() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void update();

    void moveTo(const Vector3& position);

    void setZoom(const float value) {
        zoom = value;
        setOrthographic(zoom);
    }

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    bool move{false};
    Vector2 mousePosOld;
    float zoom{100.0f};
    float zoomMin{5.0f};
    float zoomMax{300.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
