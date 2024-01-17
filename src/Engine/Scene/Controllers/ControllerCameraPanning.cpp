#include "ControllerCameraPanning.hpp"

using namespace Engine;

ControllerCameraPanning::ControllerCameraPanning(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerCameraPanning::~ControllerCameraPanning() = default;

void ControllerCameraPanning::update(const float delta) {
    auto entities = reg.view<ComponentCameraPanning>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.update(delta);
    }
}

void ControllerCameraPanning::recalculate(VulkanRenderer& vulkan) {
}

void ControllerCameraPanning::eventMouseMoved(const Vector2i& pos) {
    auto entities = reg.view<ComponentCameraPanning>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseMoved(pos);
    }
}

void ControllerCameraPanning::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    auto entities = reg.view<ComponentCameraPanning>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMousePressed(pos, button);
    }
}

void ControllerCameraPanning::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto entities = reg.view<ComponentCameraPanning>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseReleased(pos, button);
    }
}

void ControllerCameraPanning::eventMouseScroll(const int xscroll, const int yscroll) {
    auto entities = reg.view<ComponentCameraPanning>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseScroll(xscroll, yscroll);
    }
}

void ControllerCameraPanning::eventKeyPressed(const Key key, const Modifiers modifiers) {
    auto entities = reg.view<ComponentCameraPanning>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventKeyPressed(key, modifiers);
    }
}

void ControllerCameraPanning::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto entities = reg.view<ComponentCameraPanning>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventKeyReleased(key, modifiers);
    }
}

void ControllerCameraPanning::eventCharTyped(const uint32_t code) {
    (void)code;
}
