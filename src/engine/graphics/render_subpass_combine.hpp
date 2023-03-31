#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassCombine : public RenderSubpass {
public:
    explicit RenderSubpassCombine(const Config& config, VulkanRenderer& vulkan, Registry& registry,
                                  const VulkanTexture& color, const VulkanTexture& blured);
    virtual ~RenderSubpassCombine() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    const Config& config;
    VulkanRenderer& vulkan;
    const VulkanTexture& color;
    const VulkanTexture& blured;
    RenderPipeline pipelineCombine;
    Mesh fullScreenQuad;
};
} // namespace Engine
