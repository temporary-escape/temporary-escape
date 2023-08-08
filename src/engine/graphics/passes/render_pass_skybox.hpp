#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_planet.hpp"
#include "../pipelines/render_pipeline_skybox.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassSkybox : public RenderPass {
public:
    explicit RenderPassSkybox(VulkanRenderer& vulkan, RenderBufferPbr& buffer, RenderResources& resources,
                              AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderSkybox(VulkanCommandBuffer& vkb, Scene& scene, const SkyboxTextures& skyboxTextures);
    void renderPlanets(VulkanCommandBuffer& vkb, Scene& scene, const SkyboxTextures& skyboxTextures);

    RenderResources& resources;
    RenderPipelineSkybox pipelineSkybox;
    RenderPipelinePlanet pipelinePlanet;
    TexturePtr textureBrdf;
};
} // namespace Engine
