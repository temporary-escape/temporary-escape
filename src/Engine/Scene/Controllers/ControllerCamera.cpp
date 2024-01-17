#include "ControllerCamera.hpp"

using namespace Engine;

ControllerCamera::ControllerCamera(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
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
