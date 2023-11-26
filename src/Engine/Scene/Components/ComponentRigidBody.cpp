#include "ComponentRigidBody.hpp"
#include "../../Server/Lua.hpp"
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

    void getWorldTransform(btTransform& worldTrans) const override {
        const auto mat = componentTransform.getAbsoluteTransform();
        /*float scaleX = glm::length(glm::vec3(mat[0][0], mat[0][1], mat[0][2]));
        float scaleY = glm::length(glm::vec3(mat[1][0], mat[1][1], mat[1][2]));
        float scaleZ = glm::length(glm::vec3(mat[2][0], mat[2][1], mat[2][2]));
        glm::mat4 matWithoutScale = glm::scale(mat, glm::vec3(1.0f / scaleX, 1.0f / scaleY, 1.0f / scaleZ));*/
        worldTrans.setFromOpenGLMatrix(&mat[0][0]);
    }

    void setWorldTransform(const btTransform& worldTrans) override {
        Matrix4 mat;
        worldTrans.getOpenGLMatrix(&mat[0][0]);
        mat = glm::scale(mat, Vector3{componentRigidBody.getScale()});
        componentTransform.setTransform(mat);
        componentRigidBody.setDirty(true);
    }

private:
    ComponentRigidBody& componentRigidBody;
    ComponentTransform& componentTransform;
};

ComponentRigidBody::ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

ComponentRigidBody::~ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(ComponentRigidBody&& other) noexcept = default;

ComponentRigidBody& ComponentRigidBody::operator=(ComponentRigidBody&& other) noexcept = default;

void ComponentRigidBody::setShape(std::unique_ptr<btCollisionShape> value) {
    auto transform = tryGet<ComponentTransform>();
    if (!transform) {
        EXCEPTION("ComponentRigidBody added on entity with no ComponentTransform");
    }

    shape = std::move(value);
    shape->setLocalScaling(btVector3{scale, scale, scale});

    if (rigidBody) {
        rigidBody->setCollisionShape(shape.get());
        btVector3 localInertia{0.0f, 0.0f, 0.0f};
        if (mass != 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }
        rigidBody->setMassProps(mass, localInertia);

    } else {
        motionState = std::unique_ptr<btMotionState>{new ComponentTransformMotionState(*this, *transform)};

        btVector3 localInertia{0.0f, 0.0f, 0.0f};
        if (mass != 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }

        btRigidBody::btRigidBodyConstructionInfo rbInfo{mass, motionState.get(), shape.get(), localInertia};
        rigidBody = std::make_unique<btRigidBody>(rbInfo);

        /*btTransform worldTransform;
        worldTransform.setFromOpenGLMatrix(&transform->getAbsoluteTransform()[0][0]);
        rigidBody->setWorldTransform(worldTransform);*/

        if (mass == 0.0f) {
            btTransform worldTransform;
            const auto m = withoutScale(transform->getAbsoluteTransform());
            worldTransform.setFromOpenGLMatrix(&m[0][0]);
            rigidBody->setWorldTransform(worldTransform);
        }

        if (kinematic) {
            logger.warn("Setting to kinematic!");
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            rigidBody->setActivationState(DISABLE_DEACTIVATION);
        }

        dynamicsWorld->addRigidBody(rigidBody.get());
    }

    setDirty(true);
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

void ComponentRigidBody::activate() {
    if (!rigidBody) {
        return;
    }
    rigidBody->activate();
}

bool ComponentRigidBody::isActive() const {
    return rigidBody && rigidBody->isActive();
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

int32_t ComponentRigidBody::getFlags() const {
    if (!rigidBody) {
        return 0;
    }
    return rigidBody->getFlags();
}

void ComponentRigidBody::setFlags(const int32_t value) {
    if (!rigidBody) {
        return;
    }
    rigidBody->setFlags(value);
}

void ComponentRigidBody::clearForces() {
    if (!rigidBody) {
        return;
    }
    rigidBody->clearForces();
}

void ComponentRigidBody::resetTransform(const Vector3& pos, const Quaternion& rot) {
    if (!rigidBody) {
        return;
    }
    Matrix4 mat{1.0f};
    mat = mat * glm::toMat4(rot);
    mat[3] = Vector4(pos, mat[3].w);
    setWorldTransform(mat);
    updateTransform();
}

void ComponentRigidBody::updateTransform() {
    if (!rigidBody) {
        return;
    }
    motionState->setWorldTransform(rigidBody->getWorldTransform());
    dynamicsWorld->updateSingleAabb(rigidBody.get());
}

void ComponentRigidBody::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentRigidBody>("ComponentRigidBody");
    cls["linear_velocity"] =
        sol::property(&ComponentRigidBody::getLinearVelocity, &ComponentRigidBody::setLinearVelocity);
    cls["angular_velocity"] =
        sol::property(&ComponentRigidBody::getAngularVelocity, &ComponentRigidBody::setAngularVelocity);
    cls["mass"] = sol::property(&ComponentRigidBody::getMass, &ComponentRigidBody::setMass);
    cls["scale"] = sol::property(&ComponentRigidBody::getScale, &ComponentRigidBody::setScale);
    cls["transform"] = sol::property(&ComponentRigidBody::getWorldTransform, &ComponentRigidBody::setWorldTransform);
    cls["kinematic"] = sol::property(&ComponentRigidBody::getKinematic, &ComponentRigidBody::setKinematic);
    cls["clear_forces"] = &ComponentRigidBody::clearForces;
    cls["activate"] = &ComponentRigidBody::activate;
    cls["reset_transform"] = &ComponentRigidBody::resetTransform;
}
