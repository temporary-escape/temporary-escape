#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API ControllerIconSelectable;

class ENGINE_API RenderSubpassCompute : public RenderSubpass {
public:
    explicit RenderSubpassCompute(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager);
    virtual ~RenderSubpassCompute() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void renderSceneCompute(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                            ControllerIconSelectable& controller);

    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipeline pipelinePositionFeedback;
};
} // namespace Engine
