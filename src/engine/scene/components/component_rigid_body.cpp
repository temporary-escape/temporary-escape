#include "component_rigid_body.hpp"
#include "../../server/lua.hpp"
#include <btBulletDynamicsCommon.h>
#include <sol/sol.hpp>

using namespace Engine;

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
        EXCEPTION("ComponentRigidBody has no model to create a shape from");
    }

    shape = std::unique_ptr<btCollisionShape>(new btSphereShape(btScalar(model->getRadius())));

    motionState = std::unique_ptr<btMotionState>{new ComponentTransformMotionState(componentTransform)};

    btScalar mass(1.f);
    btVector3 localInertia(0, 0, 0);

    btRigidBody::btRigidBodyConstructionInfo rbInfo{mass, motionState.get(), shape.get(), localInertia};
    rigidBody = std::make_unique<btRigidBody>(rbInfo);
}

void ComponentRigidBody::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentRigidBody>("ComponentRigidBody");
    cls["set_from_model"] = &ComponentRigidBody::setFromModel;
}

void ComponentRigidBody::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentRigidBody>(handle);
}
