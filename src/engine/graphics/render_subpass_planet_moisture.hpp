#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassPlanetMoisture : public RenderSubpass {
public:
    using Rng = std::mt19937_64;

    explicit RenderSubpassPlanetMoisture(VulkanRenderer& vulkan, RenderResources& resources,
                                         AssetsManager& assetsManager);
    virtual ~RenderSubpassPlanetMoisture() = default;

    void render(VulkanCommandBuffer& vkb, Rng& rng, int index, float resolution);

private:
    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipeline pipelineMoisture;
};
} // namespace Engine
