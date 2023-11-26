#pragma once

#include "ControllerRigidBody.hpp"

namespace Engine {
class ENGINE_API ControllerBullets : public Controller {
public:
    explicit ControllerBullets(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld);
    ~ControllerBullets() override;
    NON_COPYABLE(ControllerBullets);
    NON_MOVEABLE(ControllerBullets);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanDoubleBuffer& getVbo() const {
        return vbo;
    }
    size_t getCount() const {
        return bullets.size();
    }

    ComponentTurret::BulletInstance& allocateBullet();

private:
    entt::registry& reg;
    ControllerDynamicsWorld& dynamicsWorld;

    std::vector<ComponentTurret::BulletInstance> bullets;
    VulkanDoubleBuffer vbo;
};
} // namespace Engine
