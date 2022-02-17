#pragma once
#include "../Utils/Exceptions.hpp"
#include "Entity.hpp"

#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API Scene {
public:
    struct Bullet {
        Vector3 pos;
        Vector3 dir;
        Color4 color;
        float time{0.0f};
        float speed{0.0f};
    };

    class EventListener {
    public:
        virtual void eventEntityAdded(const EntityPtr& entity) {
            (void)entity;
        }
    };

    static inline EventListener defaultEventListener;

    explicit Scene(EventListener& eventListener = defaultEventListener);
    virtual ~Scene();

    void update(float delta);
    void removeEntity(const EntityPtr& entity);
    void addEntity(EntityPtr entity);
    void updateEntity(const Entity::Delta& delta);
    void addBullet(const Vector3& pos, const Vector3& dir);
    const std::vector<Bullet>& getBulletsData() const {
        return bullets.data;
    }
    /*template <typename T, typename... Args> T& addComponent(const EntityPtr& entity, Args&&... args) {
        auto& system = getComponentSystem<T>();
        auto& component = system.add(*entity, std::forward<Args>(args)...);
        entity->addComponent<T>(component);
        return component;
    }*/

    template <typename T> ComponentSystem<T>& getComponentSystem() const {
        const auto ptr = dynamic_cast<ComponentSystem<T>*>(systems.at(EntityComponentHelper::getIndexOf<T>()).get());
        if (!ptr) {
            EXCEPTION("The component system for {} does not exist", typeid(T).name());
        }
        return *ptr;
    }

    const std::vector<EntityPtr>& getEntities() const {
        return entities;
    }

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);

private:
    struct BulletSystem {
        std::vector<Bullet> data;
        size_t next{0};
    };

    EventListener& eventListener;
    uint64_t nextId;
    ComponentSystemsMap systems;
    std::vector<EntityPtr> entities;
    std::unordered_map<uint64_t, EntityPtr> entityMap;
    BulletSystem bullets;
};
} // namespace Engine
