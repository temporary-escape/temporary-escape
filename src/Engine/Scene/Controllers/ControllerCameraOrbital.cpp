#include "ControllerCameraOrbital.hpp"

using namespace Engine;

ControllerCameraOrbital::ControllerCameraOrbital(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerCameraOrbital::~ControllerCameraOrbital() = default;

void ControllerCameraOrbital::update(const float delta) {
    auto entities = reg.view<ComponentCameraOrbital>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.update(delta);
    }
}

void ControllerCameraOrbital::recalculate(VulkanRenderer& vulkan) {
}

void ControllerCameraOrbital::eventMouseMoved(const Vector2i& pos) {
    auto entities = reg.view<ComponentCameraOrbital>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseMoved(pos);
    }
}

void ControllerCameraOrbital::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    auto entities = reg.view<ComponentCameraOrbital>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMousePressed(pos, button);
    }
}

void ControllerCameraOrbital::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto entities = reg.view<ComponentCameraOrbital>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseReleased(pos, button);
    }
}

void ControllerCameraOrbital::eventMouseScroll(const int xscroll, const int yscroll) {
    auto entities = reg.view<ComponentCameraOrbital>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseScroll(xscroll, yscroll);
    }
}

void ControllerCameraOrbital::eventKeyPressed(const Key key, const Modifiers modifiers) {
    auto entities = reg.view<ComponentCameraOrbital>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventKeyPressed(key, modifiers);
    }
}

void ControllerCameraOrbital::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto entities = reg.view<ComponentCameraOrbital>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventKeyReleased(key, modifiers);
    }
}

void ControllerCameraOrbital::eventCharTyped(const uint32_t code) {
    (void)code;
}
