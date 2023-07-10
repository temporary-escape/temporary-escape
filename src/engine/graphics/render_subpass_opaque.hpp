#pragma once

#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassOpaque : public RenderSubpass {
public:
    explicit RenderSubpassOpaque(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                 VoxelShapeCache& voxelShapeCache);
    virtual ~RenderSubpassOpaque() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void renderSceneGrids(VulkanCommandBuffer& vkb, Scene& scene);
    void renderSceneModels(VulkanCommandBuffer& vkb, Scene& scene);
    void renderScenePlanets(VulkanCommandBuffer& vkb, Scene& scene);
    void renderSceneModelsStatic(VulkanCommandBuffer& vkb, Scene& scene);

    VulkanRenderer& vulkan;
    RenderResources& resources;
    VoxelShapeCache& voxelShapeCache;
    RenderPipeline pipelineGrid;
    RenderPipeline pipelineModel;
    RenderPipeline pipelineModelInstanced;
    RenderPipeline pipelinePlanet;
    TexturePtr palette;
};
} // namespace Engine
