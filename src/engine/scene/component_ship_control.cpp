#include "component_ship_control.hpp"
#include "../utils/exceptions.hpp"
#include "entity.hpp"
#include <glm/gtx/euler_angles.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

/*void ComponentShipControl::initParticles(const EntityPtr& entity, const ComponentGrid& grid) {
    // For all block types in the grid
    for (size_t i = 0; i < grid.getTypes().size(); i++) {
        const auto& type = grid.getTypes().at(i);

        // If the block has particle asset associated
        if (type.block->getParticles().has_value()) {
            const auto particles = type.block->getParticles().value();

            // Add the particles for all blocks in the grid for that type
            for (const auto& block : grid.getNodes()) {
                if (block.data.type == i) {
                    auto p = entity->addComponent<ComponentParticleEmitter>(particles.asset);
                    p->setOffset(block.data.pos + particles.offset);
                }
            }
        }
    }
}*/

void ComponentShipControl::setMovement(bool left, bool right, bool up, bool down) {
    moving = left || right || up || down;
    rotate[0] = left;
    rotate[1] = right;
    rotate[2] = up;
    rotate[3] = down;
}

std::tuple<float, float, float> ComponentShipControl::getAngles() const {
    if (!componentTransform) {
        return {0, 0, 0};
    }
    auto& transform = componentTransform->getTransform();

    const auto forwardDir = Vector3{transform* Vector4{0.0f, 0.0f, -1.0f, 0.0f}};
    const float pitch = glm::degrees(glm::acos(forwardDir.y / glm::length(forwardDir)));
    const float yaw = glm::degrees(glm::atan(forwardDir.x / forwardDir.z));
    const Quaternion rotation = glm::quat_cast(transform);
    const auto roll = glm::degrees(glm::roll(rotation));
    return {pitch, yaw, roll};
}

void ComponentShipControl::update(const float delta) {
    if (!componentTransform) {
        return;
    }
    // entity->translate(Vector3{0.0f, 0.0f, -10.0f * delta});
    // entity->rotate(Vector3{0.0f, 1.0f, 0.0f}, 10.0f * delta);

    // Vector3 angles;
    // glm::extractEulerAngleXYZ(entity->getTransform(), angles.x, angles.y, angles.z);

    // entity->rotate(Vector3{1.0f, 0.0f, 0.0f}, 2.0f * delta);

    auto [pitch, yaw, roll] = getAngles();

    // If the ship is not moving, correct the pitch to align to the XZ plane
    if (!moving) {
        if (pitch >= 90.0f) {
            auto angle = 2.0f * delta;
            if (pitch - 90.0f < angle) {
                angle = pitch - 90.0f;
            }
            componentTransform->rotate(Vector3{1.0f, 0.0f, 0.0f}, angle);
        } else {
            auto angle = 2.0f * delta;
            if (90.0f - pitch < angle) {
                angle = 90.0f - pitch;
            }
            componentTransform->rotate(Vector3{1.0f, 0.0f, 0.0f}, -angle);
        }
    }

    if (rotate[0]) {
        const auto angle = 20.0f * delta;
        componentTransform->rotate(Vector3{0.0f, 1.0f, 0.0f}, angle);
    }
    if (rotate[1]) {
        const auto angle = -20.0f * delta;
        componentTransform->rotate(Vector3{0.0f, 1.0f, 0.0f}, angle);
    }
    if (rotate[2]) {
        auto angle = 20.0f * delta;
        if (pitch - angle > 45.0f) {
            componentTransform->rotate(Vector3{1.0f, 0.0f, 0.0f}, angle);
        }
    }
    if (rotate[3]) {
        auto angle = -20.0f * delta;
        if (pitch - angle < 45.0f + 90.0f) {
            componentTransform->rotate(Vector3{1.0f, 0.0f, 0.0f}, angle);
        }
    }

    if (roll >= 0.0f) {
        auto angle = 15.0f * delta;
        if (angle > roll) {
            angle = roll;
        }
        componentTransform->rotate(Vector3{0.0f, 0.0f, 1.0f}, -angle * 0.5f);
    } else {
        auto angle = -15.0f * delta;
        if (angle < roll) {
            angle = roll;
        }
        componentTransform->rotate(Vector3{0.0f, 0.0f, 1.0f}, -angle * 0.5f);
    }
}
