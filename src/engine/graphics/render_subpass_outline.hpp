#pragma once

#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassOutline : public RenderSubpass {
public:
    explicit RenderSubpassOutline(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                  const VulkanTexture& entityColorTexture);
    virtual ~RenderSubpassOutline() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    VulkanRenderer& vulkan;
    RenderResources& resources;
    const VulkanTexture& entityColorTexture;
    RenderPipeline pipelineOutline;
};
} // namespace Engine
