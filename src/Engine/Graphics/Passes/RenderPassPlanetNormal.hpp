#pragma once

#include "../../Assets/PlanetType.hpp"
#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelinePlanetNormal.hpp"
#include "../RenderBufferPlanet.hpp"
#include "../RenderPass.hpp"

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
