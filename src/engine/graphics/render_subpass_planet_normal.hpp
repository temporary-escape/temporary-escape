#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassPlanetNormal : public RenderSubpass {
public:
    explicit RenderSubpassPlanetNormal(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderSubpassPlanetNormal() = default;

    void render(VulkanCommandBuffer& vkb, const VulkanTexture& heightmapTexture, float resolution,
                const PlanetTypePtr& planetType);

private:
    VulkanRenderer& vulkan;

    RenderPipeline pipelineNormal;
    Mesh fullScreenQuad;
};
} // namespace Engine
