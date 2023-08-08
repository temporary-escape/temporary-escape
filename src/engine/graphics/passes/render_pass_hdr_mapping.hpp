#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_hdr_mapping.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassHDRMapping : public RenderPass {
public:
    explicit RenderPassHDRMapping(VulkanRenderer& vulkan, RenderBufferPbr& buffer, RenderResources& resources,
                                  AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineHDRMapping pipelineHDRMapping;
};
} // namespace Engine
