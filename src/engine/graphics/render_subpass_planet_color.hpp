#pragma once

#include "../assets/planet_type.hpp"
#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderPassPlanetSurface;

class ENGINE_API RenderSubpassPlanetColor : public RenderSubpass {
public:
    using Rng = std::mt19937_64;

    explicit RenderSubpassPlanetColor(VulkanRenderer& vulkan, AssetsManager& assetsManager,
                                      RenderPassPlanetSurface& parent);
    virtual ~RenderSubpassPlanetColor() = default;

    void render(VulkanCommandBuffer& vkb, const PlanetTypePtr& planetType);

private:
    VulkanRenderer& vulkan;
    RenderPassPlanetSurface& parent;

    RenderPipeline pipelineColor;
    Mesh fullScreenQuad;
};
} // namespace Engine
