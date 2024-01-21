#pragma once

#include "Entity.hpp"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btIDebugDraw;
class btRigidBody;
class btCollisionObject;
class btDynamicsWorld;
class btTransform;

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

class ENGINE_API DynamicsWorld {
public:
    class RayCastResult {
    public:
        Vector3 hitPos;
        bool valid{false};

        operator bool() const {
            return valid;
        }
    };

    explicit DynamicsWorld(Scene& scene, entt::registry& reg, const Config& config);
    virtual ~DynamicsWorld();
    NON_COPYABLE(DynamicsWorld);
    NON_MOVEABLE(DynamicsWorld);

    void update(float delta);
    void recalculate(VulkanRenderer& vulkan);
    bool contactTest(ContactTestObject& object, CollisionMask mask = CollisionGroup::Everything) const;
    bool contactTestBox(const Vector3& origin, float width, CollisionMask mask = CollisionGroup::Everything) const;
    bool contactTestSphere(const Vector3& origin, float radius, CollisionMask mask = CollisionGroup::Everything) const;
    void updateAabbs();
    void rayCast(const Vector3& start, const Vector3& end, RayCastResult& result);

    const VulkanDoubleBuffer& getDebugDrawVbo() const;
    size_t getDebugDrawCount() const;

    btDynamicsWorld& get();

private:
    Scene& scene;
    entt::registry& reg;
    const Config& config;
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
    std::unique_ptr<btIDebugDraw> debugDraw;
};
} // namespace Engine
