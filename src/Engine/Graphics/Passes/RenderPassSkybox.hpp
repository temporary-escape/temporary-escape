#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineSkybox.hpp"
#include "../Pipelines/RenderPipelineSkyboxPlanet.hpp"
#include "../Pipelines/RenderPipelineStarFlare.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassSkybox : public RenderPass {
public:
    explicit RenderPassSkybox(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                              RenderResources& resources);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderSkybox(VulkanCommandBuffer& vkb, Scene& scene, const SkyboxTextures& skyboxTextures);
    void renderPlanets(VulkanCommandBuffer& vkb, Scene& scene, const SkyboxTextures& skyboxTextures);
    void renderStarFlare(VulkanCommandBuffer& vkb, Scene& scene);

    RenderResources& resources;
    RenderPipelineSkybox pipelineSkybox;
    RenderPipelineSkyboxPlanet pipelinePlanet;
    RenderPipelineStarFlare pipelineStarFlare;
};
} // namespace Engine
