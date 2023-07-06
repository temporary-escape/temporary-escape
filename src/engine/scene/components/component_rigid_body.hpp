#pragma once

#include "../../assets/model.hpp"
#include "component_transform.hpp"

class btRigidBody;
class btCollisionShape;
class btMotionState;

namespace Engine {
class ENGINE_API ComponentRigidBody : public Component {
public:
    ComponentRigidBody();
    explicit ComponentRigidBody(entt::registry& reg, entt::entity handle);
    virtual ~ComponentRigidBody(); // NOLINT(modernize-use-override)
    ComponentRigidBody(const ComponentRigidBody& other) = delete;
    ComponentRigidBody(ComponentRigidBody&& other) noexcept;
    ComponentRigidBody& operator=(const ComponentRigidBody& other) = delete;
    ComponentRigidBody& operator=(ComponentRigidBody&& other) noexcept;
    static constexpr auto in_place_delete = true;

    void update(ComponentTransform& componentTransform);

    btRigidBody* getRigidBody() const {
        return rigidBody.get();
    }

    void setFromModel(const ModelPtr& value);

    void setLinearVelocity(const Vector3& value);

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Component), model);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    ModelPtr model;
    std::unique_ptr<btCollisionShape> shape;
    std::unique_ptr<btMotionState> motionState;
    std::unique_ptr<btRigidBody> rigidBody;
};
} // namespace Engine
