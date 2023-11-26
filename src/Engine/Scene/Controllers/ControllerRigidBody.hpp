#pragma once

#include "ControllerDynamicsWorld.hpp"

namespace Engine {
class ENGINE_API ControllerRigidBody : public Controller {
public:
    explicit ControllerRigidBody(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld);
    ~ControllerRigidBody() override;
    NON_COPYABLE(ControllerRigidBody);
    NON_MOVEABLE(ControllerRigidBody);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    void onConstruct(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    ControllerDynamicsWorld& dynamicsWorld;
};
} // namespace Engine
