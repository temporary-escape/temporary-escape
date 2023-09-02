#pragma once

#include "controller_rigid_body.hpp"

namespace Engine {
class ENGINE_API ControllerTurret : public Controller {
public:
    explicit ControllerTurret(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld);
    ~ControllerTurret() override;
    NON_COPYABLE(ControllerTurret);
    NON_MOVEABLE(ControllerTurret);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanDoubleBuffer& getVboBullets() const {
        return vboBullets;
    }
    size_t getBulletsCount() const {
        return bullets.size();
    }

    // void receiveBullets(const msgpack::object& obj);

private:
    ComponentTurret::BulletInstance& allocateBullet();
    void onDestroyTransform(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    ControllerDynamicsWorld& dynamicsWorld;

    std::vector<ComponentTurret::BulletInstance> bullets;
    VulkanDoubleBuffer vboBullets;
};
} // namespace Engine
