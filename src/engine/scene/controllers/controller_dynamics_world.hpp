#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btIDebugDraw;

namespace Engine {
class ENGINE_API ControllerDynamicsWorld : public Controller {
public:
    explicit ControllerDynamicsWorld(entt::registry& reg, const Config& config);
    ~ControllerDynamicsWorld() override;
    NON_COPYABLE(ControllerDynamicsWorld);
    NON_MOVEABLE(ControllerDynamicsWorld);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    bool contactTestSphere(const Vector3& origin, float radius) const;

    const VulkanBuffer& getDebugDrawVbo() const;
    size_t getDebugDrawCount() const;

private:
    void onConstruct(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    const Config& config;
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btIDebugDraw> debugDraw;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
};
} // namespace Engine
