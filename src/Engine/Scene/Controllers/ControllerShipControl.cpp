#include "ControllerShipControl.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerShipControl::ControllerShipControl(entt::registry& reg) : reg{reg} {
}

ControllerShipControl::~ControllerShipControl() = default;

void ControllerShipControl::update(const float delta) {
    const auto& entities = reg.view<ComponentTransform, ComponentShipControl>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, component] : entities) {
        component.update(reg, delta, transform);
    }
}

void ControllerShipControl::recalculate(VulkanRenderer& vulkan) {
}
