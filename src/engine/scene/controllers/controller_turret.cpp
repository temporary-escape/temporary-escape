#include "controller_turret.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerTurret::ControllerTurret(entt::registry& reg) : reg{reg} {
    reg.on_destroy<ComponentTransform>().disconnect<&ControllerTurret::onDestroyTransform>(this);
}

ControllerTurret::~ControllerTurret() {
    reg.on_destroy<ComponentTransform>().disconnect<&ControllerTurret::onDestroyTransform>(this);
}

void ControllerTurret::update(const float delta) {
    const auto& entities =
        reg.view<ComponentTransform, ComponentTurret, ComponentModelSkinned>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, turret, model] : entities) {
        if (turret.isActive()) {
            turret.update(delta, transform, model);
        }
    }
}

void ControllerTurret::recalculate(VulkanRenderer& vulkan) {
}

void ControllerTurret::onDestroyTransform(entt::registry& r, entt::entity handle) {
    (void)r;

    const auto& transform = reg.get<ComponentTransform>(handle);
    for (const auto&& [_, turret] : reg.view<ComponentTurret>().each()) {
        if (turret.getTarget() == &transform) {
            turret.setTarget(nullptr);
        }
    }
}
