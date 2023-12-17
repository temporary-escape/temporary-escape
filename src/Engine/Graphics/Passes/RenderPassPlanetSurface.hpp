#pragma once

#include "../../Assets/PlanetType.hpp"
#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelinePlanetColor.hpp"
#include "../Pipelines/RenderPipelinePlanetHeight.hpp"
#include "../Pipelines/RenderPipelinePlanetMoisture.hpp"
#include "../RenderBufferPlanet.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassPlanetSurface : public RenderPass {
public:
    explicit RenderPassPlanetSurface(VulkanRenderer& vulkan, RenderBufferPlanet& buffer, RenderResources& resources);

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
