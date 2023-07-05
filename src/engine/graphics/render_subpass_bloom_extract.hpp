#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassBloomExtract : public RenderSubpass {
public:
    explicit RenderSubpassBloomExtract(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                       const VulkanTexture& source);
    virtual ~RenderSubpassBloomExtract() = default;

    void render(VulkanCommandBuffer& vkb);

private:
    VulkanRenderer& vulkan;
    RenderResources& resources;
    const VulkanTexture& source;
    RenderPipeline pipelineBloomExtract;
};
} // namespace Engine
