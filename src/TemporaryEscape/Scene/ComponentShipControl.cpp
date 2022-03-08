#include "ComponentShipControl.hpp"
#include "../Utils/Exceptions.hpp"
#include "Entity.hpp"
#include <glm/gtx/euler_angles.hpp>

#define CMP "ComponentShipControl"

using namespace Engine;

void ComponentShipControl::init() {
    auto entity = getEntity();

    if (const auto grid = entity->findComponent<ComponentGrid>()) {
        // initParticles(entity, *grid);
    }
}

void ComponentShipControl::initParticles(const EntityPtr& entity, const ComponentGrid& grid) {
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
}

std::tuple<float, float, float> ComponentShipControl::getAngles() const {
    auto entity = getEntity();
    const auto forwardDir = Vector3{entity->getTransform() * Vector4{0.0f, 0.0f, -1.0f, 0.0f}};
    const float pitch = glm::degrees(glm::acos(forwardDir.y / glm::length(forwardDir)));
    const float yaw = glm::degrees(glm::atan(forwardDir.x / forwardDir.z));
    const Quaternion rotation = glm::quat_cast(entity->getTransform());
    const auto roll = glm::degrees(glm::roll(rotation));
    return {pitch, yaw, roll};
}

void ComponentShipControl::update(const float delta) {
    auto entity = getEntity();
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
            entity->rotate(Vector3{1.0f, 0.0f, 0.0f}, angle);
        } else {
            auto angle = 2.0f * delta;
            if (90.0f - pitch < angle) {
                angle = 90.0f - pitch;
            }
            entity->rotate(Vector3{1.0f, 0.0f, 0.0f}, -angle);
        }
    }

    if (rotate[0]) {
        const auto angle = 20.0f * delta;
        entity->rotate(Vector3{0.0f, 1.0f, 0.0f}, angle);
    }
    if (rotate[1]) {
        const auto angle = -20.0f * delta;
        entity->rotate(Vector3{0.0f, 1.0f, 0.0f}, angle);
    }
    if (rotate[2]) {
        auto angle = 20.0f * delta;
        if (pitch - angle > 45.0f) {
            entity->rotate(Vector3{1.0f, 0.0f, 0.0f}, angle);
        }
    }
    if (rotate[3]) {
        auto angle = -20.0f * delta;
        if (pitch - angle < 45.0f + 90.0f) {
            entity->rotate(Vector3{1.0f, 0.0f, 0.0f}, angle);
        }
    }

    if (roll >= 0.0f) {
        auto angle = 15.0f * delta;
        if (angle > roll) {
            angle = roll;
        }
        entity->rotate(Vector3{0.0f, 0.0f, 1.0f}, -angle * 0.5f);
    } else {
        auto angle = -15.0f * delta;
        if (angle < roll) {
            angle = roll;
        }
        entity->rotate(Vector3{0.0f, 0.0f, 1.0f}, -angle * 0.5f);
    }
}

std::shared_ptr<Entity> ComponentShipControl::getEntity() const {
    auto ptr = dynamic_cast<Entity*>(&getObject());
    if (!ptr) {
        EXCEPTION("Failed to cast object to an entity. This should never happen.");
    }
    return ptr->shared_from_this();
}
