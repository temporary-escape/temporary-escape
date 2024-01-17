#include "ComponentCameraPanning.hpp"
#include "ComponentTransform.hpp"

using namespace Engine;

ComponentCameraPanning::ComponentCameraPanning(EntityId entity, ComponentCamera& camera) :
    Component{entity}, camera{&camera} {
}

void ComponentCameraPanning::update(const float delta) {
    zoomValue = lerp(zoomValue, zoomTarget, 8.0f * delta);
    camera->setOrthographic(zoomValue);
}

void ComponentCameraPanning::setTarget(const Vector3& value) {
    camera->lookAt(value, value - Vector3{0.0f, 1.0f, 0.0f}, Vector3{0.0f, 0.0f, 1.0f});
}

void ComponentCameraPanning::setDistance(const float value) {
    zoomTarget = value;
    zoomValue = value;
}

void ComponentCameraPanning::setDistanceRange(const float min, const float max) {
    zoomRange = {min, max};
}

void ComponentCameraPanning::eventMouseMoved(const Vector2i& pos) {
    if (panFlag) {
        panning = true;

        const auto from = camera->screenToWorld(mousePosOld);
        const auto to = camera->screenToWorld(pos);

        const auto diff = Vector3{from.x - to.x, 0.0f, from.z - to.z};
        setTarget(Vector3{camera->getTransform()[3]} + diff * Vector3{1.0f, 1.0f, -1.0f});

        mousePosOld = pos;
    }
}

void ComponentCameraPanning::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (button == MouseButton::Right && !panFlag) {
        panFlag = true;
        mousePosOld = pos;
    }
}

void ComponentCameraPanning::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (button == MouseButton::Right) {
        panFlag = false;
        panning = false;
    }
}

void ComponentCameraPanning::eventMouseScroll(const int xscroll, const int yscroll) {
    const auto factor = map(zoomTarget, zoomRange.x, zoomRange.y, 2.0f, 200.0f);
    zoomTarget += static_cast<float>(-yscroll * 0.2f) * factor;

    if (zoomTarget < zoomRange.x) {
        zoomTarget = zoomRange.x;
    } else if (zoomTarget > zoomRange.y) {
        zoomTarget = zoomRange.y;
    }
}

void ComponentCameraPanning::eventKeyPressed(const Key key, const Modifiers modifiers) {
}

void ComponentCameraPanning::eventKeyReleased(const Key key, const Modifiers modifiers) {
}

void ComponentCameraPanning::eventCharTyped(uint32_t code) {
}
