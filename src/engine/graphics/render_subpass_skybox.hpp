#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassSkybox : public RenderSubpass {
public:
    explicit RenderSubpassSkybox(VulkanRenderer& vulkan, Registry& registry, const VulkanTexture& brdf);
    virtual ~RenderSubpassSkybox() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void renderSkybox(VulkanCommandBuffer& vkb, Scene& scene);
    void renderPlanets(VulkanCommandBuffer& vkb, Scene& scene);
    void renderPlanet(VulkanCommandBuffer& vkb, const ComponentCamera& camera, Skybox& skybox,
                      ComponentTransform& transform, ComponentPlanet& component);
    void updateDirectionalLights(Scene& scene);

    VulkanRenderer& vulkan;
    const VulkanTexture& brdf;
    RenderPipeline pipelineSkybox;
    RenderPipeline pipelinePlanetSurface;
    Mesh cube;
    Mesh planet;
    VulkanDoubleBuffer directionalLightsUbo;
};
} // namespace Engine
