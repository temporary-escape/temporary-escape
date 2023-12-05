#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btIDebugDraw;
class btRigidBody;
class btCollisionObject;

namespace Engine {
class ContactTestObject {
public:
    ContactTestObject(CollisionShape& shape);
    ~ContactTestObject();

    btCollisionObject* get() const;
    void setTransform(const Matrix4& value);

private:
    std::unique_ptr<btCollisionShape> shape;
    std::unique_ptr<btCollisionObject> object;
};

class ENGINE_API ControllerDynamicsWorld : public Controller {
public:
    class RayCastResult {
    public:
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
    bool contactTest(ContactTestObject& object, CollisionMask mask = CollisionGroup::Everything) const;
    bool contactTestBox(const Vector3& origin, float width, CollisionMask mask = CollisionGroup::Everything) const;
    bool contactTestSphere(const Vector3& origin, float radius, CollisionMask mask = CollisionGroup::Everything) const;
    void updateAabbs();
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
