#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerModelSkinned : public Controller {
public:
    struct Item {
        entt::entity entity;
        Model* model;
        Matrix4 transform;
        size_t offset;
    };

    explicit ControllerModelSkinned(Scene& scene, entt::registry& reg);
    ~ControllerModelSkinned() override;
    NON_COPYABLE(ControllerModelSkinned);
    NON_MOVEABLE(ControllerModelSkinned);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    [[nodiscard]] const VulkanDescriptorSet& getDescriptorSet() const;

private:
    void addOrUpdate(entt::entity handle, ComponentModelSkinned& component);
    void remove(entt::entity handle);

    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    Scene& scene;
    entt::registry& reg;
    VulkanArrayBuffer buffer;
    VulkanRenderer* device{nullptr};
    VulkanDescriptorPool descriptorPool;
    VulkanDescriptorSetLayout descriptorSetLayout;
    std::array<VulkanDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets;
};
} // namespace Engine
