#include "controller_camera.hpp"

using namespace Engine;

ControllerCamera::ControllerCamera(entt::registry& reg) : reg{reg} {
}

ControllerCamera::~ControllerCamera() = default;

void ControllerCamera::update(const float delta) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.update(delta);
    }
}

void ControllerCamera::recalculate(VulkanRenderer& vulkan) {
}

void ControllerCamera::eventMouseMoved(const Vector2i& pos) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseMoved(pos);
    }
}

void ControllerCamera::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMousePressed(pos, button);
    }
}

void ControllerCamera::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseReleased(pos, button);
    }
}

void ControllerCamera::eventMouseScroll(const int xscroll, const int yscroll) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventMouseScroll(xscroll, yscroll);
    }
}

void ControllerCamera::eventKeyPressed(const Key key, const Modifiers modifiers) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventKeyPressed(key, modifiers);
    }
}

void ControllerCamera::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.eventKeyReleased(key, modifiers);
    }
}

void ControllerCamera::eventCharTyped(const uint32_t code) {
    (void)code;
}
