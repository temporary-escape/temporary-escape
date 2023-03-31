#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassCompute : public RenderSubpass {
public:
    explicit RenderSubpassCompute(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderSubpassCompute() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void renderSceneCompute(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentClickablePoints& component);

    VulkanRenderer& vulkan;
    RenderPipeline pipelinePositionFeedback;
};
} // namespace Engine
