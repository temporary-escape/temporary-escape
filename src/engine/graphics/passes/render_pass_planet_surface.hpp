#pragma once

#include "../../assets/planet_type.hpp"
#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_planet_color.hpp"
#include "../pipelines/render_pipeline_planet_height.hpp"
#include "../pipelines/render_pipeline_planet_moisture.hpp"
#include "../render_buffer_planet.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassPlanetSurface : public RenderPass {
public:
    explicit RenderPassPlanetSurface(VulkanRenderer& vulkan, RenderBufferPlanet& buffer, RenderResources& resources,
                                     AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;
    void setSeed(uint64_t value);
    void setIndex(int value);
    void setPlanetType(PlanetTypePtr value);

private:
    void renderHeight(VulkanCommandBuffer& vkb);
    void renderMoisture(VulkanCommandBuffer& vkb);
    void renderColor(VulkanCommandBuffer& vkb);

    RenderBufferPlanet& buffer;
    RenderResources& resources;
    RenderPipelinePlanetHeight pipelinePlanetHeight;
    RenderPipelinePlanetMoisture pipelinePlanetMoisture;
    RenderPipelinePlanetColor pipelinePlanetColor;
    std::mt19937_64 rng;
    int index{0};
    PlanetTypePtr planetType{nullptr};
};
} // namespace Engine
