#include "component_rigid_body.hpp"
#include "../../server/lua.hpp"
#include <btBulletDynamicsCommon.h>
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class ComponentTransformMotionState : public btMotionState {
public:
    explicit ComponentTransformMotionState(ComponentRigidBody& componentRigidBody,
                                           ComponentTransform& componentTransform) :
        componentRigidBody{componentRigidBody}, componentTransform{componentTransform} {
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
        componentRigidBody.setDirty(true);
    }

    ComponentRigidBody& componentRigidBody;
    ComponentTransform& componentTransform;
};

ComponentRigidBody::ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

ComponentRigidBody::ComponentRigidBody(entt::registry& reg, entt::entity handle, const ModelPtr& model) :
    Component{reg, handle}, model{model} {
    setup();
}

ComponentRigidBody::~ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(ComponentRigidBody&& other) noexcept = default;

ComponentRigidBody& ComponentRigidBody::operator=(ComponentRigidBody&& other) noexcept = default;

void ComponentRigidBody::setModel(const ModelPtr& value) {
    if (model == value) {
        return;
    }
    model = value;
}

const ModelPtr& ComponentRigidBody::getModel() const {
    return model;
}

void ComponentRigidBody::setup() {
    if (!model) {
        EXCEPTION("ComponentRigidBody setup with no model");
    }
    if (!model->getCollisionShape()) {
        EXCEPTION("ComponentRigidBody setup with a model: '{}' that has no collision shape", model->getName());
    }
    auto transform = tryGet<ComponentTransform>();
    if (!transform) {
        EXCEPTION("ComponentRigidBody added on entity with no ComponentTransform");
    }

    shape = std::make_unique<btUniformScalingShape>(model->getCollisionShape(), scale);

    motionState = std::unique_ptr<btMotionState>{new ComponentTransformMotionState(*this, *transform)};

    btVector3 localInertia{0.0f, 0.0f, 0.0f};
    if (mass != 0.0f) {
        shape->calculateLocalInertia(mass, localInertia);
    }

    btRigidBody::btRigidBodyConstructionInfo rbInfo{mass, motionState.get(), shape.get(), localInertia};
    rigidBody = std::make_unique<btRigidBody>(rbInfo);
}

void ComponentRigidBody::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentRigidBody>(handle);
}

void ComponentRigidBody::setMass(const float value) {
    if (mass == value) {
        return;
    }

    mass = value;

    if (rigidBody) {
        btVector3 localInertia{0.0f, 0.0f, 0.0f};
        if (mass != 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }
        rigidBody->setMassProps(mass, localInertia);

        auto flags = rigidBody->getCollisionFlags();
        if (mass != 0.0f) {
            flags &= ~(btCollisionObject::CF_STATIC_OBJECT);
            flags |= btCollisionObject::CF_DYNAMIC_OBJECT;
            rigidBody->setCollisionFlags(flags);
        } else {
            flags &= ~(btCollisionObject::CF_DYNAMIC_OBJECT);
            flags |= btCollisionObject::CF_STATIC_OBJECT;
            rigidBody->setCollisionFlags(flags);
        }
    }
}

float ComponentRigidBody::getMass() const {
    return mass;
}

void ComponentRigidBody::setScale(float value) {
    if (scale == value) {
        return;
    }

    scale = value;

    if (rigidBody) {
        shape->setLocalScaling(btVector3{scale, scale, scale});
        if (dynamicsWorld) {
            dynamicsWorld->updateSingleAabb(rigidBody.get());
        }
    }
}

float ComponentRigidBody::getScale() const {
    return scale;
}

void ComponentRigidBody::setLinearVelocity(const Vector3& value) {
    if (!rigidBody) {
        return;
    }
    rigidBody->setLinearVelocity({value.x, value.y, value.z});
}

void ComponentRigidBody::activate() {
    rigidBody->activate();
}

Vector3 ComponentRigidBody::getLinearVelocity() const {
    if (!rigidBody) {
        return Vector3{0.0f};
    }
    const auto& vel = rigidBody->getLinearVelocity();
    return {vel.x(), vel.y(), vel.z()};
}

void ComponentRigidBody::setAngularVelocity(const Vector3& value) {
    if (!rigidBody) {
        return;
    }
    rigidBody->setAngularVelocity({value.x, value.y, value.z});
}

Vector3 ComponentRigidBody::getAngularVelocity() const {
    if (!rigidBody) {
        return Vector3{0.0f};
    }
    const auto& vel = rigidBody->getAngularVelocity();
    return {vel.x(), vel.y(), vel.z()};
}

void ComponentRigidBody::setWorldTransform(const Matrix4& value) {
    if (!rigidBody) {
        return;
    }
    btTransform transform{};
    transform.setFromOpenGLMatrix(&value[0][0]);
    rigidBody->setWorldTransform(transform);
}

Matrix4 ComponentRigidBody::getWorldTransform() const {
    if (!rigidBody) {
        return Matrix4{0.0f};
    }
    Matrix4 mat;
    rigidBody->getWorldTransform().getOpenGLMatrix(&mat[0][0]);
    return mat;
}

void ComponentRigidBody::clearForces() {
    if (!rigidBody) {
        return;
    }
    rigidBody->clearForces();
}

void ComponentRigidBody::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentRigidBody>("ComponentRigidBody");
    cls["model"] = sol::readonly_property(&ComponentRigidBody::getModel);
    cls["linear_velocity"] =
        sol::property(&ComponentRigidBody::getLinearVelocity, &ComponentRigidBody::setLinearVelocity);
    cls["angular_velocity"] =
        sol::property(&ComponentRigidBody::getAngularVelocity, &ComponentRigidBody::setAngularVelocity);
    cls["mass"] = sol::property(&ComponentRigidBody::getMass, &ComponentRigidBody::setMass);
    cls["transform"] = sol::property(&ComponentRigidBody::getWorldTransform, &ComponentRigidBody::setWorldTransform);
    cls["clear_forces"] = &ComponentRigidBody::clearForces;
    cls["activate"] = &ComponentRigidBody::activate;
}
