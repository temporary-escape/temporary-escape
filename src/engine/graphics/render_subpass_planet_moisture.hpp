#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassPlanetMoisture : public RenderSubpass {
public:
    using Rng = std::mt19937_64;

    explicit RenderSubpassPlanetMoisture(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderSubpassPlanetMoisture() = default;

    void render(VulkanCommandBuffer& vkb, Rng& rng, int index, float resolution);

private:
    VulkanRenderer& vulkan;

    RenderPipeline pipelineMoisture;
    Mesh fullScreenQuad;
};
} // namespace Engine
