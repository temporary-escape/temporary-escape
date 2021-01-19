#include "Scene.hpp"

#include "../Graphics/Renderer.hpp"
#include "ComponentCamera.hpp"
#include "ComponentGrid.hpp"
#include "ComponentModel.hpp"

using namespace Scissio;

uint64_t Scene::nextId = 1;

Scene::Scene()
    : systems{
          {ComponentModel::Type, std::make_shared<ComponentSystem<ComponentModel>>()},
          {ComponentCamera::Type, std::make_shared<ComponentSystem<ComponentCamera>>()},
          {ComponentGrid::Type, std::make_shared<ComponentSystem<ComponentGrid>>()},
      } {
}

Scene::~Scene() {
}

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

void Scene::render(Renderer& renderer) {
    for (auto& entity : entitiesAdded) {
        for (auto& component : entity->getComponents()) {
            auto& system = systems.at(component->getType());
            system->add(*component.get());
        }
        entities.push_back(entity);
    }

    entitiesAdded.clear();

    auto& systemModel = getComponentSystem<ComponentModel>();
    auto& systemCamera = getComponentSystem<ComponentCamera>();
    auto& systemGrid = getComponentSystem<ComponentGrid>();

    glEnable(GL_DEPTH_TEST);
    for (const auto& component : systemModel) {
        const auto& transform = component->getObject().getTransform();
        if (component->getModel()) {
            renderer.renderModel(*component->getModel(), transform);
        }
    }

    for (const auto& component : systemGrid) {
        const auto& transform = component->getObject().getTransform();
        if (component->isDirtyClear()) {
            component->rebuildBuffers();
        }

        for (const auto& pair : component->getMeshes()) {
            const auto& data = pair.second;
            renderer.renderGridModel(data.primitives, transform);
        }
    }

    glDisable(GL_DEPTH_TEST);
}
