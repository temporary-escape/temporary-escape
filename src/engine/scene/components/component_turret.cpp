#include "component_turret.hpp"
#include "../../server/lua.hpp"
#include "../entity.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentTurret::ComponentTurret(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

ComponentTurret::ComponentTurret(entt::registry& reg, entt::entity handle, TurretPtr turret) : Component{reg, handle} {
    setTurret(std::move(turret));
}

void ComponentTurret::update(const float delta, const ComponentTransform& transform, ComponentModelSkinned& model) {
    if (!active) {
        return;
    }

    const auto absoluteTransform = transform.getAbsoluteTransform();

    // Transform target world position into local coordinate system (to cancel our rotation).
    const auto transformInverted = glm::inverse(absoluteTransform);
    const auto targetLocal = Vector3{transformInverted * Vector4{target, 1.0f}};
    const auto targetDir = glm::normalize(targetLocal /* - turret->getComponents().cannon.offset*/);

    // Calculate yaw and pitch.
    const auto yaw = std::atan2(-targetDir.z, targetDir.x) + glm::radians(90.0f);
    const auto dist = std::sqrt(targetDir.z * targetDir.z + targetDir.x * targetDir.x);
    const auto pitch = -std::atan2(targetDir.y, dist);

    model.setAdjustment(1, glm::mat4_cast(Quaternion{Vector3{0.0f, yaw, 0.0f}}));
    model.setAdjustment(2, glm::mat4_cast(Quaternion{Vector3{pitch, 0.0f, 0.0f}}));
    model.setDirty(true);

    // logger.info("Target dir: {}", targetDir);

    /*const auto absoluteTransform = getObject().getAbsoluteTransform();

    // Transform target world position into local coordinate system (to cancel our rotation).
    const auto transformInverted = glm::inverse(absoluteTransform);
    const auto targetLocal = Vector3{transformInverted * Vector4{target, 1.0f}};
    const auto targetDir = glm::normalize(targetLocal - turret->getComponents().cannon.offset);

    // Calculate yaw and pitch.
    const auto yaw = std::atan2(-targetDir.z, targetDir.x);
    const auto dist = std::sqrt(targetDir.z * targetDir.z + targetDir.x * targetDir.x);
    const auto pitch = std::atan2(targetDir.y, dist);

    // Cache the rotation values, we will need this for rendering the turret models.
    rotation = {pitch, yaw - glm::radians(90.0f), 0.0f};

    // Calculate the barrel's end position in world coordinates.
    // We will use this to calculate where the bullets should go from.
    static const auto front = Vector3{0.0f, 0.0f, -1.0f};
    auto transform = glm::translate(absoluteTransform, turret->getComponents().cannon.offset);
    transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, rotation.x, Vector3{1.0f, 0.0f, 0.0f});
    transform = glm::translate(transform, front);

    nozzlePos = transform[3];
    nozzleDir = glm::normalize(target - nozzlePos);

    if (firing) {
        counter -= delta;
    } else {
        counter = 0.0f;
    }*/
}

void ComponentTurret::setTurret(TurretPtr value) {
    turret = std::move(value);
}

void ComponentTurret::setTarget(const Vector3& value) {
    active = true;
    target = value;
}

void ComponentTurret::clearTarget() {
}

void ComponentTurret::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentTurret>("ComponentTurret");
    cls["turret"] = sol::property(&ComponentTurret::getTurret, &ComponentTurret::setTurret);
    cls["target"] = sol::property(&ComponentTurret::getTarget, &ComponentTurret::setTarget);
    cls["clear_target"] = &ComponentTurret::clearTarget;
}
