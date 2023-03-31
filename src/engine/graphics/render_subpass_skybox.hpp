#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassSkybox : public RenderSubpass {
public:
    explicit RenderSubpassSkybox(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderSubpassSkybox() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    VulkanRenderer& vulkan;
    RenderPipeline pipelineSkybox;
    Mesh cube;
};
} // namespace Engine
