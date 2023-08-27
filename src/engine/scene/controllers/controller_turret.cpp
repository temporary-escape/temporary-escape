#include "controller_turret.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerTurret::ControllerTurret(entt::registry& reg) : reg{reg} {
}

ControllerTurret::~ControllerTurret() = default;

void ControllerTurret::update(const float delta) {
    const auto& entities =
        reg.view<ComponentTransform, ComponentTurret, ComponentModelSkinned>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, turret, model] : entities) {
        turret.update(delta, transform, model);
    }
}

void ControllerTurret::recalculate(VulkanRenderer& vulkan) {
}
