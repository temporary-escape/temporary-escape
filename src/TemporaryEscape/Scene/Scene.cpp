#include "Scene.hpp"
#include "../Assets/SkyboxRenderer.hpp"

#define CMP "Scene"

using namespace Engine;

Scene::Scene(EventListener& eventListener)
    : eventListener(eventListener), nextId(0), systems{EntityComponentHelper::generateComponentSystemsMap()} {
}

Scene::~Scene() = default;

void Scene::addEntity(EntityPtr entity) {
    if (entity->getId() == 0) {
        entity->setId(++nextId);
    } else {
        nextId = std::max(entity->getId(), nextId);
    }

    if (entity->getParentId()) {
        const auto found = entityMap.find(entity->getParentId());
        if (found != entityMap.end()) {
            entity->setParent(found->second);
        } else {
            Log::w(CMP, "Entity parent id: {} not found", entity->getParentId());
        }
    }

    entities.push_back(std::move(entity));
    entityMap.insert(std::make_pair(entities.back()->getId(), entities.back()));

    for (const auto& ref : entities.back()->getComponents()) {
        auto& system = systems.at(ref.type);
        system->add(*ref.ptr);
    }

    eventListener.eventEntityAdded(entities.back());

    for (const auto& child : entities.back()->getChildren()) {
        addEntity(child);
    }
}

void Scene::updateEntity(const Entity::Delta& delta) {
    auto it = entityMap.find(delta.id);
    if (it != entityMap.end()) {
        it->second->applyDelta(delta);
    }
}

void Scene::removeEntity(const EntityPtr& entity) {
    if (!entity) {
        return;
    }

    for (const auto& child : entity->getChildren()) {
        child->setParent(nullptr);
        removeEntity(child);
    }
    entity->clearChildren();
    entity->setParent(nullptr);
    if (entity->hasComponent<ComponentScript>()) {
        entity->getComponent<ComponentScript>()->clear();
    }

    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
    entityMap.erase(entity->getId());
    for (const auto& ref : entity->getComponents()) {
        auto& system = systems.at(ref.type);
        system->remove(*ref.ptr);
    }
}

EntityPtr Scene::getEntityById(uint64_t id) {
    const auto it = entityMap.find(id);
    if (it == entityMap.end()) {
        return nullptr;
    }
    return it->second;
}

void Scene::addBullet(const Vector3& pos, const Vector3& dir) {
    const auto add = [&](Bullet& bullet) {
        bullet.pos = pos;
        bullet.dir = dir;
        bullet.time = 1.0f;
        bullet.color = Color4{1.0f};
        bullet.speed = 125.0f;
    };

    for (auto& bullet : bullets.data) {
        if (bullet.speed <= 0.0001f) {
            add(bullet);
            return;
        }
    }

    const auto endIdx = bullets.data.size();
    Log::d(CMP, "Resizing bullets buffer to {}", bullets.data.size() + 128);
    bullets.data.resize(bullets.data.size() + 128);
    add(bullets.data.at(endIdx));
}

void Scene::update(const float delta) {
    auto& systemCameraTurntable = getComponentSystem<ComponentCameraTurntable>();
    for (auto& component : systemCameraTurntable) {
        component->update();
    }

    auto& systemCameraTop = getComponentSystem<ComponentCameraTop>();
    for (auto& component : systemCameraTop) {
        component->update();
    }

    auto& systemShipControl = getComponentSystem<ComponentShipControl>();
    for (auto& component : systemShipControl) {
        component->update(delta);
    }

    auto& systemTurret = getComponentSystem<ComponentTurret>();
    for (auto& component : systemTurret) {
        component->update(delta);
        if (component->shouldFire()) {
            component->resetFire();
            addBullet(component->getNozzlePos(), component->getNozzleDir());
        }
    }

    for (auto& bullet : bullets.data) {
        if (bullet.speed <= 0.0001f) {
            continue;
        }

        bullet.time += delta;
        bullet.pos += bullet.dir * bullet.speed * delta;

        if (bullet.time > 10.0f) {
            bullet.speed = 0.0f;
        }
    }
}

std::shared_ptr<Camera> Scene::getPrimaryCamera() const {
    if (auto ptr = primaryCamera.lock(); ptr != nullptr) {
        const auto top = ptr->findComponent<ComponentCameraTop>();
        if (top) {
            return top;
        }
        const auto turntable = ptr->findComponent<ComponentCameraTurntable>();
        if (turntable) {
            return turntable;
        }
    }

    return nullptr;
}

std::optional<std::reference_wrapper<const Skybox>> Scene::getSkybox(SkyboxRenderer& renderer) {
    auto& skyboxSystem = getComponentSystem<ComponentSkybox>();
    if (skyboxSystem.begin() != skyboxSystem.end()) {
        auto& component = **skyboxSystem.begin();
        component.recalculate(renderer);
        return component.getSkybox();
    }
    return std::nullopt;
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
