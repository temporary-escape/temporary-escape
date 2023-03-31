#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassBrdf : public RenderSubpass {
public:
    explicit RenderSubpassBrdf(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderSubpassBrdf() = default;

    void render(VulkanCommandBuffer& vkb);

private:
    VulkanRenderer& vulkan;
    RenderPipeline pipelineBrdf;
    Mesh fullScreenQuad;
};
} // namespace Engine
