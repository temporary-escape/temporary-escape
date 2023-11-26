#pragma once

#include "ControllerBullets.hpp"
#include "ControllerRigidBody.hpp"

namespace Engine {
class ENGINE_API ControllerTurret : public Controller {
public:
    explicit ControllerTurret(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld, ControllerBullets& bullets);
    ~ControllerTurret() override;
    NON_COPYABLE(ControllerTurret);
    NON_MOVEABLE(ControllerTurret);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

private:
    void onDestroyTransform(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    ControllerDynamicsWorld& dynamicsWorld;
    ControllerBullets& bullets;
};
} // namespace Engine
