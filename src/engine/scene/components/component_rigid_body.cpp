#include "component_rigid_body.hpp"
#include "../../server/lua.hpp"
#include <btBulletDynamicsCommon.h>
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class ComponentTransformMotionState : public btMotionState {
public:
    explicit ComponentTransformMotionState(ComponentTransform& componentTransform) :
        componentTransform{componentTransform} {
    }

    ~ComponentTransformMotionState() override = default;

private:
    void getWorldTransform(btTransform& worldTrans) const override {
        worldTrans.setFromOpenGLMatrix(&componentTransform.getAbsoluteTransform()[0][0]);
    }

    void setWorldTransform(const btTransform& worldTrans) override {
        Matrix4 mat;
        worldTrans.getOpenGLMatrix(&mat[0][0]);
        componentTransform.updateTransform(mat);
    }

    ComponentTransform& componentTransform;
};

ComponentRigidBody::ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

ComponentRigidBody::~ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(ComponentRigidBody&& other) noexcept = default;

ComponentRigidBody& ComponentRigidBody::operator=(ComponentRigidBody&& other) noexcept = default;

void ComponentRigidBody::setFromModel(const ModelPtr& value) {
    model = value;
    setDirty(true);
}

void ComponentRigidBody::update(ComponentTransform& componentTransform) {
    if (!model) {
        logger.error("ComponentRigidBody created with no model");
        return;
    }
    if (!model->getCollisionShape()) {
        logger.error("ComponentRigidBody created with a model that has no collision shape");
        return;
    }

    // shape = std::unique_ptr<btCollisionShape>(new btSphereShape(btScalar(model->getRadius())));
    shape = std::make_unique<btUniformScalingShape>(model->getCollisionShape(), 1.0f);

    motionState = std::unique_ptr<btMotionState>{new ComponentTransformMotionState(componentTransform)};

    btScalar mass{1.0f};
    btVector3 localInertia{0.0f, 0.0f, 0.0f};
    shape->calculateLocalInertia(mass, localInertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo{mass, motionState.get(), shape.get(), localInertia};
    rigidBody = std::make_unique<btRigidBody>(rbInfo);
}

void ComponentRigidBody::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentRigidBody>(handle);
}

void ComponentRigidBody::setLinearVelocity(const Vector3& value) {
    if (!rigidBody) {
        return;
    }
    rigidBody->setLinearVelocity({value.x, value.y, value.z});
}

void ComponentRigidBody::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentRigidBody>("ComponentRigidBody");
    cls["set_from_model"] = &ComponentRigidBody::setFromModel;
    cls["set_linear_velocity"] = &ComponentRigidBody::setLinearVelocity;
}
