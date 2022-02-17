#include "ComponentShipControl.hpp"
#include "../Utils/Exceptions.hpp"
#include "Entity.hpp"

using namespace Engine;

void ComponentShipControl::init() {
    auto entity = getEntity();

    if (const auto cmp = entity->getComponentOpt<ComponentGrid>(); cmp.has_value()) {
        const auto& grid = cmp.value();
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

void ComponentShipControl::update(const float delta) {
    auto entity = getEntity();
    entity->translate(Vector3{0.0f, 0.0f, -10.0f * delta});
    entity->rotate(Vector3{0.0f, 1.0f, 0.0f}, 10.0f * delta);
}

std::shared_ptr<Entity> ComponentShipControl::getEntity() const {
    auto ptr = dynamic_cast<Entity*>(&getObject());
    if (!ptr) {
        EXCEPTION("Failed to cast object to an entity. This should never happen.");
    }
    return ptr->shared_from_this();
}
