#include "ComponentTurret.hpp"
#include "../Scene.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentTurret::ComponentTurret(EntityId entity) : Component{entity} {
}

ComponentTurret::ComponentTurret(EntityId entity, TurretPtr turret) : Component{entity} {
    setTurret(std::move(turret));
}

void ComponentTurret::update(Scene& scene, const float delta, const ComponentTransform& transform,
                             ComponentModelSkinned& model) {
    if (!active) {
        return;
    }

    if (target) {
        targetPos = target->getAbsolutePosition();
        scene.setDirty(*this);
    }

    const auto absoluteTransform = transform.getAbsoluteTransform();
    const auto globalPosition = Vector3{absoluteTransform[3]};

    // Transform target world position into local coordinate system (to cancel our rotation).
    const auto transformInverted = glm::inverse(absoluteTransform);
    const auto targetLocal = Vector3{transformInverted * Vector4{targetPos, 1.0f}};
    const auto targetDir = glm::normalize(targetLocal /* - turret->getComponents().cannon.offset*/);

    targetDirGlobal = glm::normalize(targetPos - globalPosition);

    // Calculate yaw and pitch.
    const auto yaw = std::atan2(-targetDir.z, targetDir.x) + glm::radians(90.0f);
    const auto dist = std::sqrt(targetDir.z * targetDir.z + targetDir.x * targetDir.x);
    const auto pitch = -std::atan2(targetDir.y, dist);

    model.setAdjustment(1, glm::mat4_cast(Quaternion{Vector3{0.0f, yaw, 0.0f}}));
    model.setAdjustment(2, glm::mat4_cast(Quaternion{Vector3{pitch, 0.0f, 0.0f}}));
    scene.setDirty(model);

    if (counter == 20 && shootReady) {
        shootReady = false;
    }

    if (counter > 0) {
        --counter;
    } else if (counter == 0 && target) {
        shootReady = true;
    }
}

bool ComponentTurret::shouldShoot() const {
    return shootReady;
}

void ComponentTurret::resetShoot() {
    counter = 20;
}

void ComponentTurret::setTurret(TurretPtr value) {
    turret = std::move(value);
}

void ComponentTurret::setTarget(const ComponentTransform* value) {
    target = value;
    if (target) {
        active = true;
    }
}

void ComponentTurret::clearTarget() {
}
