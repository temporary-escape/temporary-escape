#pragma once
#include "../Utils/Exceptions.hpp"
#include "Entity.hpp"

#include <unordered_map>
#include <vector>

namespace Scissio {
class SCISSIO_API Scene {
public:
    using ComponentSystemsMap = std::unordered_map<ComponentType, std::shared_ptr<AbstractComponentSystem>>;

    explicit Scene();
    virtual ~Scene();
    Scene(const Scene& other) = delete;
    Scene(Scene&& other) noexcept;
    Scene& operator=(const Scene& other) = delete;
    Scene& operator=(Scene&& other) noexcept;
    void swap(Scene& other) noexcept;

    void update();

    EntityPtr addEntity();
    void insertEntity(EntityPtr entity); // AbstractClient only!
    /*template <typename T, typename... Args> T& addComponent(const EntityPtr& entity, Args&&... args) {
        auto& system = getComponentSystem<T>();
        auto& component = system.add(*entity, std::forward<Args>(args)...);
        entity->addComponent<T>(component);
        return component;
    }*/

    template <typename T> ComponentSystem<T>& getComponentSystem() const {
        const auto ptr = dynamic_cast<ComponentSystem<T>*>(systems.at(T::Type).get());
        if (!ptr) {
            EXCEPTION("The component system for {} does not exist", typeid(T).name());
        }
        return *ptr;
    }

    const std::vector<EntityPtr>& getEntities() const {
        return entities;
    }

private:
    static uint64_t nextId;

    ComponentSystemsMap systems;
    std::vector<EntityPtr> entitiesAdded;
    std::vector<EntityPtr> entities;
};
} // namespace Scissio
