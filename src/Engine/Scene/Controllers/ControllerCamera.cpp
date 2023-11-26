#include "ControllerCamera.hpp"

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
