#pragma once

#include "../Components/ComponentParticles.hpp"
#include "../Controller.hpp"
#include "../DynamicsWorld.hpp"

namespace Engine {
class ENGINE_API ControllerGrid : public Controller {
public:
    static constexpr size_t particlesBatchSize{256};

    struct ParticlesBatch {
        VulkanDoubleBuffer uniforms;
        std::array<const VulkanDescriptorSet*, particlesBatchSize> descriptorSetsTypes{};
        std::array<uint32_t, particlesBatchSize> descriptorSetsIndexes{};
        std::array<int, particlesBatchSize> counts{};
        VulkanDescriptorPool descriptorPool;
        VulkanDescriptorSetLayout descriptorSetLayout;
        std::array<VulkanDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSetsBatch;
        size_t count{0};
    };

    explicit ControllerGrid(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld,
                            VoxelShapeCache* voxelShapeCache);
    ~ControllerGrid() override;
    NON_COPYABLE(ControllerGrid);
    NON_MOVEABLE(ControllerGrid);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    [[nodiscard]] const ParticlesBatch& getParticlesBatch() const {
        return particlesBatch;
    }

private:
    void addOrUpdate(entt::entity handle, ComponentGrid& component);
    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);
    void addThrustParticles(VulkanRenderer& vulkan, const ComponentParticles::ParticlesBatchUniform& uniform,
                            const ParticlesTypePtr& particlesType);

    Scene& scene;
    entt::registry& reg;
    DynamicsWorld& dynamicsWorld;
    VoxelShapeCache* voxelShapeCache;
    ParticlesBatch particlesBatch;
};
} // namespace Engine
