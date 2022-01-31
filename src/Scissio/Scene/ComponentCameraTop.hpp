#pragma once

#include "Camera.hpp"
#include "ComponentUserInput.hpp"

namespace Scissio {
class SCISSIO_API ComponentCameraTop : public Component, public Camera, public UserInputHandler {
public:
    ComponentCameraTop() = default;
    explicit ComponentCameraTop(Object& object) : Component(object), Camera(object) {
    }

    virtual ~ComponentCameraTop() = default;

    void update() {
        setOrthographic(zoom);
        moveTo(getObject().getPosition());
    }

    void moveTo(const Vector3& position) {
        lookAt(position + Vector3{0.0f, 1.0f, 0.0f}, position, Vector3{0.0f, 0.0f, 1.0f});
    }

    void setZoom(const float value) {
        zoom = value;
    }

    void eventMouseMoved(const Vector2i& pos) override {
        if (move) {
            const auto from = screenToWorld(mousePosOld);
            const auto to = screenToWorld(pos);

            const auto diff = Vector3{from.x - to.x, 0.0f, from.z - to.z};
            moveTo(getObject().getPosition() + diff);

            mousePosOld = pos;
        }
    }

    void eventMousePressed(const Vector2i& pos, const MouseButton button) override {
        mousePosOld = pos;
        move |= button == MouseButton::Right;
    }

    void eventMouseReleased(const Vector2i& pos, const MouseButton button) override {
        mousePosOld = pos;
        move &= !(button == MouseButton::Right);
    }

    void eventMouseScroll(const int xscroll, const int yscroll) override {
        const auto factor = map(zoom, zoomMin, zoomMax, 2.0f, 15.0f);
        zoom += static_cast<float>(-yscroll) * factor;

        if (zoom < zoomMin) {
            zoom = zoomMin;
        } else if (zoom > zoomMax) {
            zoom = zoomMax;
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
    bool primary{true};
    bool move{false};
    Vector2 mousePosOld;
    float zoom{100.0f};
    float zoomMin{5.0f};
    float zoomMax{300.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Scissio
