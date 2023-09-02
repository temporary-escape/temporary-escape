#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btIDebugDraw;
class btRigidBody;

namespace Engine {
class ENGINE_API ControllerDynamicsWorld : public Controller {
public:
    struct RayCastResult {
        Vector3 hitPos;
        bool valid{false};

        operator bool() const {
            return valid;
        }
    };

    explicit ControllerDynamicsWorld(entt::registry& reg, const Config& config);
    ~ControllerDynamicsWorld() override;
    NON_COPYABLE(ControllerDynamicsWorld);
    NON_MOVEABLE(ControllerDynamicsWorld);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;
    bool contactTestSphere(const Vector3& origin, float radius) const;
    void rayCast(const Vector3& start, const Vector3& end, RayCastResult& result);

    const VulkanDoubleBuffer& getDebugDrawVbo() const;
    size_t getDebugDrawCount() const;

    btDynamicsWorld& get();

private:
    const Config& config;
    entt::registry& reg;
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
    std::unique_ptr<btIDebugDraw> debugDraw;
};
} // namespace Engine
