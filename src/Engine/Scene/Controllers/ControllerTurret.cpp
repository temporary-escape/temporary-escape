#include "ControllerTurret.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerTurret::ControllerTurret(Scene& scene, entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld,
                                   ControllerBullets& bullets) :
    scene{scene}, reg{reg}, dynamicsWorld{dynamicsWorld}, bullets{bullets} {
    reg.on_destroy<ComponentTransform>().disconnect<&ControllerTurret::onDestroyTransform>(this);
}

ControllerTurret::~ControllerTurret() {
    reg.on_destroy<ComponentTransform>().disconnect<&ControllerTurret::onDestroyTransform>(this);
}

void ControllerTurret::update(const float delta) {
    const auto& entities =
        reg.view<ComponentTransform, ComponentTurret, ComponentModelSkinned>(entt::exclude<TagDisabled>).each();
    for (const auto&& [entity, transform, turret, model] : entities) {
        if (turret.isActive()) {
            turret.update(scene, delta, transform, model);

            if (turret.shouldShoot()) {
                turret.resetShoot();
                auto& bullet = bullets.allocateBullet();
                bullet.origin = transform.getAbsolutePosition();
                bullet.lifetime = 10.0f;
                bullet.direction = turret.getTargetDirection();
                bullet.size = 10.0f;
                bullet.speed = 500.0f;
            }
        }
    }
}

void ControllerTurret::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
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
