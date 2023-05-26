#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassPlanetHeightmap : public RenderSubpass {
public:
    using Rng = std::mt19937_64;

    explicit RenderSubpassPlanetHeightmap(VulkanRenderer& vulkan, AssetsManager& assetsManager);
    virtual ~RenderSubpassPlanetHeightmap() = default;

    void render(VulkanCommandBuffer& vkb, Rng& rng, int index, float resolution);

private:
    VulkanRenderer& vulkan;

    RenderPipeline pipelineHeightmap;
    Mesh fullScreenQuad;
};
} // namespace Engine
