#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_skybox_nebula.hpp"
#include "../pipelines/render_pipeline_skybox_stars.hpp"
#include "../render_buffer_skybox.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxColor : public RenderPass {
public:
    explicit RenderPassSkyboxColor(VulkanRenderer& vulkan, RenderBufferSkybox& buffer, RenderResources& resources,
                                   AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderNebula(VulkanCommandBuffer& vkb, Scene& scene);
    void renderPointClouds(VulkanCommandBuffer& vkb, Scene& scene);

    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipelineSkyboxNebula pipelineSkyboxNebula;
    RenderPipelineSkyboxStars pipelineSkyboxStars;
};
} // namespace Engine
