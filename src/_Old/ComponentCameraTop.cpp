#include "ComponentCameraTop.hpp"

using namespace Engine;

void ComponentCameraTop::update() {
    setZoom(zoom);
    moveTo(getEyesPos());
}

void ComponentCameraTop::moveTo(const Vector3& position) {
    lookAt(position, position - Vector3{0.0f, 1.0f, 0.0f}, Vector3{0.0f, 0.0f, 1.0f});
}

void ComponentCameraTop::eventMouseMoved(const Vector2i& pos) {
    if (move) {
        const auto from = screenToWorld(mousePosOld);
        const auto to = screenToWorld(pos);

        const auto diff = Vector3{from.x - to.x, 0.0f, from.z - to.z};
        moveTo(getObject().getPosition() + diff);

        mousePosOld = pos;
    }
}

void ComponentCameraTop::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    move |= button == MouseButton::Right;
}

void ComponentCameraTop::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    move &= !(button == MouseButton::Right);
}

void ComponentCameraTop::eventMouseScroll(const int xscroll, const int yscroll) {
    const auto factor = map(zoom, zoomMin, zoomMax, 2.0f, 15.0f);
    zoom += static_cast<float>(-yscroll) * factor;

    if (zoom < zoomMin) {
        zoom = zoomMin;
    } else if (zoom > zoomMax) {
        zoom = zoomMax;
    }

    setOrthographic(zoom);
}

void ComponentCameraTop::eventKeyPressed(const Key key, const Modifiers modifiers) {
}

void ComponentCameraTop::eventKeyReleased(const Key key, const Modifiers modifiers) {
}
