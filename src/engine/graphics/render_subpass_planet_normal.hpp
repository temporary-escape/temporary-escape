#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassPlanetNormal : public RenderSubpass {
public:
    explicit RenderSubpassPlanetNormal(VulkanRenderer& vulkan, RenderResources& resources,
                                       AssetsManager& assetsManager);
    virtual ~RenderSubpassPlanetNormal() = default;

    void render(VulkanCommandBuffer& vkb, const VulkanTexture& heightmapTexture, float resolution,
                const PlanetTypePtr& planetType);

private:
    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipeline pipelineNormal;
};
} // namespace Engine
