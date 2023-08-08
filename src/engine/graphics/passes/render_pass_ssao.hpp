#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_ssao.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassSSAO : public RenderPass {
public:
    explicit RenderPassSSAO(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                            RenderResources& resources, AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    const RenderOptions& options;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineSSAO pipelineSSAO;
};
} // namespace Engine