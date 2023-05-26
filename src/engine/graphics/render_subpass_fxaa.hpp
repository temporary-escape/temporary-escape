#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassFxaa : public RenderSubpass {
public:
    explicit RenderSubpassFxaa(VulkanRenderer& vulkan, AssetsManager& assetsManager, const VulkanTexture& forward);
    virtual ~RenderSubpassFxaa() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    VulkanRenderer& vulkan;
    const VulkanTexture& forward;
    RenderPipeline pipelineFxaa;
    Mesh fullScreenQuad;
};
} // namespace Engine
