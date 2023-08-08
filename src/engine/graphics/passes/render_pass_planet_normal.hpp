#pragma once

#include "../../assets/planet_type.hpp"
#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_planet_normal.hpp"
#include "../render_buffer_planet.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassPlanetNormal : public RenderPass {
public:
    explicit RenderPassPlanetNormal(VulkanRenderer& vulkan, RenderBufferPlanet& buffer, RenderResources& resources,
                                    AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;
    void setPlanetType(PlanetTypePtr value);

private:
    void renderNormal(VulkanCommandBuffer& vkb);

    RenderBufferPlanet& buffer;
    RenderResources& resources;
    RenderPipelinePlanetNormal pipelinePlanetNormal;
    PlanetTypePtr planetType{nullptr};
};
} // namespace Engine
