#include "scene.hpp"
#include "../assets/registry.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

Scene::Scene(EventListener& eventListener) : eventListener{eventListener}, nextId{0} {
}

Scene::~Scene() = default;
 
void Scene::addEntity(EntityPtr entity) {
    entities.push_back(std::move(entity));
}

void Scene::removeEntity(const EntityPtr& entity) {
    if (!entity) {
        return;
    }

    entity->destroy();
    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());

    /*for (const auto& child : entity->getChildren()) {
        child->setParent(nullptr);
        removeEntity(child);
    }
    entity->clearChildren();
    entity->setParent(nullptr);
    if (entity->hasComponent<ComponentScript>()) {
        // entity->getComponent<ComponentScript>()->clear();
    }

    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
    entityMap.erase(entity->getId());
    for (const auto& ref : entity->getComponents()) {
        auto& system = systems.at(ref.type);
        system->remove(*ref.ptr);
    }*/
}

EntityPtr Scene::getEntityById(uint64_t id) {
    const auto it = entityMap.find(id);
    if (it == entityMap.end()) {
        return nullptr;
    }
    return it->second;
}

void Scene::addBullet(const Vector3& pos, const Vector3& dir) {
    /*const auto add = [&](Bullet& bullet) {
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
    logger.debug("Resizing bullets buffer to {}", bullets.data.size() + 128);
    bullets.data.resize(bullets.data.size() + 128);
    add(bullets.data.at(endIdx));*/
}

void Scene::update(const float delta) {
    for (auto&& [entity, camera] : getView<ComponentCamera>().each()) {
        camera.update(delta);
    }

    /*auto& systemCamera = getComponentSystem<ComponentCamera>();
    for (auto& component : systemCamera) {
        component->update(delta);
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
    }*/
}

ComponentCamera* Scene::getPrimaryCamera() {
    if (auto ptr = primaryCamera.lock(); ptr != nullptr) {
        if (!ptr->hasComponent<ComponentCamera>()) {
            return nullptr;
        }

        return &ptr->getComponent<ComponentCamera>();
    }

    return nullptr;
}

void Scene::eventMouseMoved(const Vector2i& pos) {
    for (auto&& [entity, input] : getView<ComponentUserInput>().each()) {
        input.eventMouseMoved(pos);
    }
}

void Scene::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    for (auto&& [entity, input] : getView<ComponentUserInput>().each()) {
        input.eventMousePressed(pos, button);
    }
}

void Scene::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    for (auto&& [entity, input] : getView<ComponentUserInput>().each()) {
        input.eventMouseReleased(pos, button);
    }
}

void Scene::eventMouseScroll(const int xscroll, const int yscroll) {
    for (auto&& [entity, input] : getView<ComponentUserInput>().each()) {
        input.eventMouseScroll(xscroll, yscroll);
    }
}

void Scene::eventKeyPressed(const Key key, const Modifiers modifiers) {
    for (auto&& [entity, input] : getView<ComponentUserInput>().each()) {
        input.eventKeyPressed(key, modifiers);
    }
}

void Scene::eventKeyReleased(const Key key, const Modifiers modifiers) {
    for (auto&& [entity, input] : getView<ComponentUserInput>().each()) {
        input.eventKeyReleased(key, modifiers);
    }
}

void Scene::eventCharTyped(const uint32_t code) {
    for (auto&& [entity, input] : getView<ComponentUserInput>().each()) {
        input.eventCharTyped(code);
    }
}

std::tuple<Vector3, Vector3> Scene::screenToWorld(const Vector2& mousePos, float length) {
    const auto camera = getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Error during screenToWorld no camera in scene");
    }

    const auto x = static_cast<float>(camera->getViewport().x) - mousePos.x; // WTF?
    const auto y = mousePos.y;
    const auto ray = camera->screenToWorld({x, y}) * -length;
    const auto eyes = camera->getEyesPos();
    return {eyes, eyes + ray};
}
