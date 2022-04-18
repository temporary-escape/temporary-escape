#include "ComponentCameraTurntable.hpp"
#include "Entity.hpp"

using namespace Engine;

void ComponentCameraTurntable::update() {
    if (const auto ptr = entity.lock(); ptr) {
        position = ptr->getPosition();
    }

    glm::mat4x4 transform{1.0f};
    transform = glm::rotate(transform, glm::radians(rotation.x), Vector3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, glm::radians(rotation.y), Vector3{1.0f, 0.0f, 0.0f});

    const auto eyes = position + Vector3(transform * Vector4(0.0f, 0.0f, zoom, 1.0f));
    lookAt(eyes, position);
}

void ComponentCameraTurntable::follow(const std::shared_ptr<Entity>& value) {
    entity = value;
}

void ComponentCameraTurntable::eventMouseMoved(const Vector2i& pos) {
    if (rotate) {
        auto newRotation = rotation + (mousePosOld - Vector2(pos)) * 0.2f;
        setRotation(newRotation);
        mousePosOld = pos;
    }
}

void ComponentCameraTurntable::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    rotate |= button == MouseButton::Right;
}

void ComponentCameraTurntable::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    rotate &= !(button == MouseButton::Right);
}

void ComponentCameraTurntable::eventMouseScroll(const int xscroll, const int yscroll) {
    const auto factor = map(zoom, 1.0f, 500.0f, 0.2f, 20.0f);
    zoom += static_cast<float>(-yscroll) * factor;

    if (zoom < 1.0f) {
        zoom = 1.0f;
    } else if (zoom > 500.0f) {
        zoom = 500.0f;
    }
}

void ComponentCameraTurntable::eventKeyPressed(const Key key, const Modifiers modifiers) {
}

void ComponentCameraTurntable::eventKeyReleased(const Key key, const Modifiers modifiers) {
}
