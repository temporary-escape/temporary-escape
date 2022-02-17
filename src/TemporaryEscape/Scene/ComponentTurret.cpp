#include "ComponentTurret.hpp"

using namespace Engine;

void ComponentTurret::update(const float delta) {
    const auto offset = getObject().getPosition();

    // Transform target world position into local coordinate system (to cancel our rotation).
    const auto transformInverted = glm::inverse(getObject().getTransform());
    const auto targetLocal = Vector3{transformInverted * Vector4{target, 1.0f}};
    const auto targetDir = glm::normalize(targetLocal - (offset + turret->getComponents().cannon.offset));

    // Calculate yaw and pitch.
    const auto yaw = std::atan2(-targetDir.z, targetDir.x);
    const auto dist = std::sqrt(targetDir.z * targetDir.z + targetDir.x * targetDir.x);
    const auto pitch = std::atan2(targetDir.y, dist);

    // Cache the rotation values, we will need this for rendering the turret models.
    rotation = {pitch, yaw - glm::radians(90.0f), 0.0f};

    // Calculate the barrel's end position in world coordinates.
    // We will use this to calculate where the bullets should go from.
    static const auto front = Vector3{0.0f, 0.0f, -1.0f};
    auto transform = glm::translate(getObject().getTransform(), offset + turret->getComponents().cannon.offset);
    transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, rotation.x, Vector3{1.0f, 0.0f, 0.0f});
    transform = glm::translate(transform, front);

    nozzlePos = transform[3];
    nozzleDir = glm::normalize(target - nozzlePos);

    if (firing) {
        counter -= delta;
    } else {
        counter = 0.0f;
    }
}
