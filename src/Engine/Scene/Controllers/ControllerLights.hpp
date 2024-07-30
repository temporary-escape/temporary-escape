#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerLights : public Controller {
public:
    struct DirectionalLightsUniform {
        Vector4 colors[4];
        Vector4 directions[4];
        int count{0};
    };

    struct ShadowsViewProj {
        Matrix4 lightMat[4];
        float cascadeSplits[4];
    };

    explicit ControllerLights(Scene& scene, entt::registry& reg);
    ~ControllerLights() override;
    NON_COPYABLE(ControllerLights);
    NON_MOVEABLE(ControllerLights);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanDoubleBuffer& getUboDirectionalLights() const {
        return uboDirectionalLights;
    }

    const VulkanDescriptorSet& getDescriptorSetShadowCamera() const;

    const VulkanDoubleBuffer& getUboShadowsViewProj() const {
        return uboShadowsViewProj;
    }

    bool hasShadows() const {
        return uboShadowReady;
    }

private:
    void updateDirectionalLights(VulkanRenderer& vulkan);
    void calculateShadowCamera(VulkanRenderer& vulkan);
    void prepareUboShadow(VulkanRenderer& vulkan);
    void prepareDescriptorSets(VulkanRenderer& vulkan);

    Scene& scene;
    entt::registry& reg;
    bool uboShadowReady{false};
    VulkanRenderer* device{nullptr};
    VulkanDoubleBuffer uboShadowCamera;
    VulkanDoubleBuffer uboShadowsViewProj;
    VulkanDoubleBuffer uboDirectionalLights;
    VulkanDescriptorPool descriptorPool;
    VulkanDescriptorSetLayout descriptorSetLayout;
    std::array<VulkanDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSetsShadowCamera;
};
} // namespace Engine
