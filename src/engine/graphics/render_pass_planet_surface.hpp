#pragma once

#include "render_pass.hpp"
#include "render_subpass_planet_color.hpp"
#include "render_subpass_planet_heightmap.hpp"
#include "render_subpass_planet_moisture.hpp"

namespace Engine {
class ENGINE_API RenderPassPlanetSurface : public RenderPass {
public:
    using Rng = std::mt19937_64;

    enum Attachments {
        Heightmap = 0,
        Moisture = 1,
        Color = 2,
        MetallicRoughness = 3,
    };

    static const size_t totalAttachments = 4;

    explicit RenderPassPlanetSurface(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                     const Vector2i& viewport);
    virtual ~RenderPassPlanetSurface() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Rng& rng, int index,
                const PlanetTypePtr& planetType);

private:
    RenderSubpassPlanetHeightmap subpassPlanetHeightmap;
    RenderSubpassPlanetMoisture subpassPlanetMoisture;
    RenderSubpassPlanetColor subpassPlanetColor;
};
} // namespace Engine
