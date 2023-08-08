#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

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

    explicit ControllerLights(entt::registry& reg, Scene& scene);
    ~ControllerLights() override;
    NON_COPYABLE(ControllerLights);
    NON_MOVEABLE(ControllerLights);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanDoubleBuffer& getUboDirectionalLights() const {
        return uboDirectionalLights;
    }

    const VulkanDoubleBuffer& getUboShadowCamera() const {
        return uboShadowCamera;
    }

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

    entt::registry& reg;
    Scene& scene;
    bool uboShadowReady{false};
    VulkanDoubleBuffer uboShadowCamera;
    VulkanDoubleBuffer uboShadowsViewProj;
    VulkanDoubleBuffer uboDirectionalLights;
};
} // namespace Engine
