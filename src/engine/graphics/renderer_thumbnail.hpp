#pragma once

#include "renderer_scene_pbr.hpp"

namespace Engine {
class ENGINE_API RendererThumbnail : public RendererScenePbr {
public:
    explicit RendererThumbnail(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                               AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache);

    void render(const BlockPtr& block, const VoxelShape::Type shape);

private:
    void renderOneTime(Scene& scene);

    const Config& config;
    VulkanRenderer& vulkan;
    VoxelShapeCache& voxelShapeCache;
    VulkanCommandBuffer vkb;
};
} // namespace Engine
