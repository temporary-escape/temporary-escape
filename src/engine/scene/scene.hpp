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

    explicit Scene(const VoxelShapeCache* voxelShapeCache = nullptr,
                   EventListener& eventListener = defaultEventListener);
    virtual ~Scene();

    std::tuple<Vector3, Vector3> screenToWorld(const Vector2& mousePos, float length);
    void update(float delta);
    void renderPbr(VulkanDevice& vulkan, const Vector2i& viewport);
    void renderFwd(VulkanDevice& vulkan, const Vector2i& viewport);
    void removeEntity(const EntityPtr& entity);
    void addEntity(EntityPtr entity);
    void updateEntity(const Entity::Delta& delta);
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

    std::shared_ptr<ComponentCamera> getPrimaryCamera() const;
    EntityPtr getSkybox();

private:
    struct BulletSystem {
        std::vector<Bullet> data;
        size_t next{0};
    };

    const VoxelShapeCache* voxelShapeCache{nullptr};
    //Pipelines* pipelines{nullptr};
    EventListener& eventListener;

    uint64_t nextId;
    ComponentSystemsMap systems;
    std::vector<EntityPtr> entities;
    std::unordered_map<uint64_t, EntityPtr> entityMap;

    BulletSystem bullets;
    EntityWeakPtr primaryCamera;
};
} // namespace Engine
