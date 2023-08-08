#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_skybox_prefilter.hpp"
#include "../render_buffer_skybox.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxPrefilter : public RenderPass {
public:
    explicit RenderPassSkyboxPrefilter(VulkanRenderer& vulkan, RenderBufferSkybox& buffer, RenderResources& resources,
                                        AssetsManager& assetsManager);

    void setTextureSkybox(const VulkanTexture& value);
    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipelineSkyboxPrefilter pipelineSkyboxPrefilter;
    const VulkanTexture* textureSkybox{nullptr};
};
} // namespace Engine
