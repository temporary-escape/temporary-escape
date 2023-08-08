#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_icons.hpp"
#include "../pipelines/render_pipeline_outline.hpp"
#include "../pipelines/render_pipeline_world_text.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassNonHDR : public RenderPass {
public:
    explicit RenderPassNonHDR(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                              RenderResources& resources, AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderOutline(VulkanCommandBuffer& vkb, Scene& scene);
    void renderIcons(VulkanCommandBuffer& vkb, Scene& scene);
    void renderWorldText(VulkanCommandBuffer& vkb, Scene& scene);

    const RenderOptions& options;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineOutline pipelineOutline;
    RenderPipelineIcons pipelineIcons;
    RenderPipelineWorldText pipelineWorldText;
};
} // namespace Engine
