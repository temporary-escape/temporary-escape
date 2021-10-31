#include "Scene.hpp"

#include "ComponentGrid.hpp"
#include "ComponentLines.hpp"
#include "ComponentModel.hpp"
#include "ComponentParticleEmitter.hpp"
#include "ComponentPointCloud.hpp"

using namespace Scissio;

uint64_t Scene::nextId = 1;

Scene::Scene()
    : systems{
          {ComponentModel::Type, std::make_shared<ComponentSystem<ComponentModel>>()},
          {ComponentGrid::Type, std::make_shared<ComponentSystem<ComponentGrid>>()},
          {ComponentPointCloud::Type, std::make_shared<ComponentSystem<ComponentPointCloud>>()},
          {ComponentLines::Type, std::make_shared<ComponentSystem<ComponentLines>>()},
          {ComponentParticleEmitter::Type, std::make_shared<ComponentSystem<ComponentParticleEmitter>>()},
      } {
}

Scene::~Scene() = default;

Scene::Scene(Scene&& other) noexcept {
    swap(other);
}

Scene& Scene::operator=(Scene&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Scene::swap(Scene& other) noexcept {
    std::swap(entities, other.entities);
    std::swap(entitiesAdded, other.entitiesAdded);
    std::swap(systems, other.systems);
}

EntityPtr Scene::addEntity() {
    entitiesAdded.push_back(std::make_shared<Entity>(nextId++));
    return entitiesAdded.back();
}

void Scene::insertEntity(EntityPtr entity) {
    entitiesAdded.push_back(std::move(entity));
}

void Scene::update() {
    for (auto& entity : entitiesAdded) {
        for (auto& component : entity->getComponents()) {
            if (component->getType() == 0)
                continue;

            auto& system = systems.at(component->getType());
            system->add(*component);
        }
        entities.push_back(entity);
    }

    entitiesAdded.clear();
}
