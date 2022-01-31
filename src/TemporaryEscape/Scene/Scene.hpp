#pragma once
#include "../Utils/Exceptions.hpp"
#include "Entity.hpp"

#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API Scene {
public:
    class EventListener {
    public:
        virtual void eventEntityAdded(const EntityPtr& entity) {
            (void)entity;
        }
    };

    static inline EventListener defaultEventListener;

    explicit Scene(EventListener& eventListener = defaultEventListener);
    virtual ~Scene();

    void update();
    void removeEntity(const EntityPtr& entity);
    void addEntity(EntityPtr entity);
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
    EventListener& eventListener;
    uint64_t nextId;
    ComponentSystemsMap systems;
    std::vector<EntityPtr> entities;
};
} // namespace Engine
