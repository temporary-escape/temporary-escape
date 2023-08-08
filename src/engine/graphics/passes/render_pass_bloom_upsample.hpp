#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_bloom_upsample.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassBloomUpsample : public RenderPass {
public:
    explicit RenderPassBloomUpsample(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                     RenderResources& resources, AssetsManager& assetsManager, uint32_t level);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineBloomUpsample pipelineBloomUpsample;
    const uint32_t level;
};
} // namespace Engine
