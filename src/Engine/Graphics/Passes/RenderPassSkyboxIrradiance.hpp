#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineSkyboxIrradiance.hpp"
#include "../RenderBufferSkybox.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxIrradiance : public RenderPass {
public:
    explicit RenderPassSkyboxIrradiance(VulkanRenderer& vulkan, RenderBufferSkybox& buffer, RenderResources& resources);

    void setTextureSkybox(const VulkanTexture& value);
    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipelineSkyboxIrradiance pipelineSkyboxIrradiance;
    const VulkanTexture* textureSkybox{nullptr};
};
} // namespace Engine
