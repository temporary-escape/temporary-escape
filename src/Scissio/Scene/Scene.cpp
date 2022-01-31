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

void Scene::removeEntity(const EntityPtr& entity) {
    const auto it = entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
    if (it != entities.end()) {
        for (const auto& componentRef : *entity) {
            auto& system = systems.at(componentRef.type);
            system->remove(*componentRef.ptr);
        }
    }
}

void Scene::update() {
    auto& systemCameraTurntable = getComponentSystem<ComponentCameraTurntable>();
    for (auto& component : systemCameraTurntable) {
        component->update();
    }

    auto& systemCameraTop = getComponentSystem<ComponentCameraTop>();
    for (auto& component : systemCameraTop) {
        component->update();
    }
}

void Scene::eventMouseMoved(const Vector2i& pos) {
    auto& componentSystemUserInput = getComponentSystem<ComponentUserInput>();
    for (auto& component : componentSystemUserInput) {
        component->eventMouseMoved(pos);
    }
}

void Scene::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    auto& componentSystemUserInput = getComponentSystem<ComponentUserInput>();
    for (auto& component : componentSystemUserInput) {
        component->eventMousePressed(pos, button);
    }
}

void Scene::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto& componentSystemUserInput = getComponentSystem<ComponentUserInput>();
    for (auto& component : componentSystemUserInput) {
        component->eventMouseReleased(pos, button);
    }
}

void Scene::eventMouseScroll(const int xscroll, const int yscroll) {
    auto& componentSystemUserInput = getComponentSystem<ComponentUserInput>();
    for (auto& component : componentSystemUserInput) {
        component->eventMouseScroll(xscroll, yscroll);
    }
}

void Scene::eventKeyPressed(const Key key, const Modifiers modifiers) {
    auto& componentSystemUserInput = getComponentSystem<ComponentUserInput>();
    for (auto& component : componentSystemUserInput) {
        component->eventKeyPressed(key, modifiers);
    }
}

void Scene::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto& componentSystemUserInput = getComponentSystem<ComponentUserInput>();
    for (auto& component : componentSystemUserInput) {
        component->eventKeyReleased(key, modifiers);
    }
}
