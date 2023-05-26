#pragma once

#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassOpaque : public RenderSubpass {
public:
    explicit RenderSubpassOpaque(VulkanRenderer& vulkan, AssetsManager& assetsManager,
                                 VoxelShapeCache& voxelShapeCache);
    virtual ~RenderSubpassOpaque() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void renderSceneGrids(VulkanCommandBuffer& vkb, Scene& scene);
    void renderSceneModels(VulkanCommandBuffer& vkb, Scene& scene);

    VulkanRenderer& vulkan;
    VoxelShapeCache& voxelShapeCache;
    RenderPipeline pipelineGrid;
    RenderPipeline pipelineModel;
    TexturePtr palette;
};
} // namespace Engine
