#include "Scene.hpp"

using namespace Scissio;

Scene::Scene(EventListener& eventListener)
    : eventListener(eventListener), nextId(0), systems{EntityComponentHelper::generateComponentSystemsMap()} {
}

Scene::~Scene() = default;

void Scene::addEntity(EntityPtr entity) {
    if (entity->getId() == 0) {
        entity->setId(++nextId);
    }

    entities.push_back(std::move(entity));

    for (const auto& componentRef : *entities.back()) {
        auto& system = systems.at(componentRef.type);
        system->add(*componentRef.ptr);
    }

    eventListener.eventEntityAdded(entities.back());
}

void Scene::update() {
}
