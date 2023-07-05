#pragma once

#include "render_pass.hpp"
#include "render_subpass_planet_normal.hpp"

namespace Engine {
class ENGINE_API RenderPassPlanetNormal : public RenderPass {
public:
    using Rng = std::mt19937_64;

    enum Attachments {
        Normal = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassPlanetNormal(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                    const Vector2i& viewport);
    virtual ~RenderPassPlanetNormal() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, const VulkanTexture& heightmapTexture,
                const PlanetTypePtr& planetType);

private:
    RenderSubpassPlanetNormal subpassPlanetNormal;
};
} // namespace Engine
