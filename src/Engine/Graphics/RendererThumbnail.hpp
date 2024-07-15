#pragma once

#include "RendererScenePbr.hpp"

namespace Engine {
class ENGINE_API RendererThumbnail : public RendererScenePbr {
public:
    explicit RendererThumbnail(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                               VoxelShapeCache& voxelShapeCache);

    void render(const BlockPtr& block, const VoxelShape::Type shape);
    void render(const PlanetTypePtr& planetType);

private:
    void renderOneTime(Scene& scene);

    const Config& config;
    VulkanRenderer& vulkan;
    VoxelShapeCache& voxelShapeCache;
    VulkanCommandBuffer vkb;
    VulkanCommandBuffer vkbc;
    VulkanSemaphore semaphore;
};
} // namespace Engine
