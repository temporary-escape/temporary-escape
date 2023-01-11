#pragma once

#include "../utils/exceptions.hpp"
#include "../vulkan/vulkan_pipeline.hpp"
#include "entity.hpp"

#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API Scene : public UserInput {
public:
    /*struct Pipelines {
        VulkanPipeline debug;
        VulkanPipeline grid;
        VulkanPipeline wireframe;
        VulkanPipeline pointCloud;
        VulkanPipeline iconPointCloud;
        VulkanPipeline lines;
    };*/

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

    std::tuple<Vector3, Vector3> screenToWorld(const Vector2& mousePos, float length);
    void update(float delta);
    void removeEntity(const EntityPtr& entity);
    EntityPtr createEntity();
    void addBullet(const Vector3& pos, const Vector3& dir);
    EntityPtr getEntityById(uint64_t id);
    const std::vector<Bullet>& getBulletsData() const {
        return bullets.data;
    }
    /*template <typename T, typename... Args> T& addComponent(const EntityPtr& entity, Args&&... args) {
        auto& system = getComponentSystem<T>();
        auto& component = system.add(*entity, std::forward<Args>(args)...);
        entity->addComponent<T>(component);
        return component;
    }*/

    template <typename T> entt::view<T> getComponentSystem() {
        return reg.view<T>();
    }

    template <typename... Ts> auto getView() {
        return reg.view<Ts...>();
    }

    const std::vector<EntityPtr>& getEntities() const {
        return entities;
    }

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    void setPrimaryCamera(const EntityPtr& entity) {
        primaryCamera = entity;
    }

    ComponentCamera* getPrimaryCamera();

private:
    struct BulletSystem {
        std::vector<Bullet> data;
        size_t next{0};
    };

    EventListener& eventListener;
    entt::registry reg;

    uint64_t nextId;
    std::vector<EntityPtr> entities;
    std::unordered_map<uint64_t, EntityPtr> entityMap;

    BulletSystem bullets;
    EntityWeakPtr primaryCamera;
};
} // namespace Engine
