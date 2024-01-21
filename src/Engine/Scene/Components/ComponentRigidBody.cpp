#include "ComponentRigidBody.hpp"
#include "../Scene.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentRigidBody::ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(EntityId entity) : Component{entity} {
}

ComponentRigidBody::~ComponentRigidBody() = default;

ComponentRigidBody::ComponentRigidBody(ComponentRigidBody&& other) noexcept = default;

ComponentRigidBody& ComponentRigidBody::operator=(ComponentRigidBody&& other) noexcept = default;

void ComponentRigidBody::setShape(ComponentTransform& transform, const CollisionShape& shape) {
    setShape(transform, shape.clone());
}

void ComponentRigidBody::setShape(ComponentTransform& transform, std::unique_ptr<btCollisionShape> value) {
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
        btVector3 localInertia{0.0f, 0.0f, 0.0f};
        if (mass != 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }

        btRigidBody::btRigidBodyConstructionInfo rbInfo{mass, &transform, shape.get(), localInertia};
        rigidBody = std::make_unique<btRigidBody>(rbInfo);

        /*btTransform worldTransform;
        worldTransform.setFromOpenGLMatrix(&transform->getAbsoluteTransform()[0][0]);
        rigidBody->setWorldTransform(worldTransform);*/

        auto collisionGroup = CollisionGroup::Default;

        if (mass == 0.0f) {
            btTransform worldTransform;
            const auto m = withoutScale(transform.getAbsoluteTransform());
            worldTransform.setFromOpenGLMatrix(&m[0][0]);
            rigidBody->setWorldTransform(worldTransform);
            collisionGroup = CollisionGroup::Static;
        }

        if (transform.isKinematic()) {
            // logger.warn("Setting to kinematic!");
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            rigidBody->setActivationState(DISABLE_DEACTIVATION);
        }

        dynamicsWorld->addRigidBody(rigidBody.get(), collisionGroup, CollisionGroup::Everything);
    }
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
    transform.setRotation(transform.getRotation().normalize());
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

void ComponentRigidBody::setBtTransform(const btTransform& value) {
    rigidBody->setWorldTransform(value);
}

const btTransform& ComponentRigidBody::getBtTransform() const {
    return rigidBody->getWorldTransform();
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

/*void ComponentRigidBody::clearForces() {
    if (!rigidBody) {
        return;
    }
    rigidBody->clearForces();
}*/

/*void ComponentRigidBody::resetTransform(const Vector3& pos, const Quaternion& rot) {
    if (!rigidBody) {
        return;
    }
    Matrix4 mat{1.0f};
    mat = mat * glm::toMat4(rot);
    mat[3] = Vector4(pos, mat[3].w);
    setWorldTransform(mat);
    updateTransform();
}*/

/*void ComponentRigidBody::updateTransform() {
    if (!rigidBody) {
        return;
    }
    motionState->setWorldTransform(rigidBody->getWorldTransform());
    dynamicsWorld->updateSingleAabb(rigidBody.get());
}*/
