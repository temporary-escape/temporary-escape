#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineSkyboxNebula.hpp"
#include "../Pipelines/RenderPipelineSkyboxStars.hpp"
#include "../RenderBufferSkybox.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxColor : public RenderPass {
public:
    explicit RenderPassSkyboxColor(VulkanRenderer& vulkan, RenderBufferSkybox& buffer, RenderResources& resources,
                                   AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderNebula(VulkanCommandBuffer& vkb, Scene& scene);
    void renderPointClouds(VulkanCommandBuffer& vkb, Scene& scene);

    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipelineSkyboxNebula pipelineSkyboxNebula;
    RenderPipelineSkyboxStars pipelineSkyboxStars;
};
} // namespace Engine
