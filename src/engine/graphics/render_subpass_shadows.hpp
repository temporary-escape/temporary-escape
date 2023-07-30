#pragma once

#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassShadows : public RenderSubpass {
public:
    explicit RenderSubpassShadows(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                  size_t index);
    virtual ~RenderSubpassShadows() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void renderSceneGrids(VulkanCommandBuffer& vkb, Scene& scene);
    void renderSceneModels(VulkanCommandBuffer& vkb, Scene& scene);
    void renderSceneModelsStatic(VulkanCommandBuffer& vkb, Scene& scene);

    VulkanRenderer& vulkan;
    RenderResources& resources;
    const size_t index;
    RenderPipeline pipelineGrid;
    RenderPipeline pipelineModel;
    RenderPipeline pipelineModelInstanced;
};
} // namespace Engine
