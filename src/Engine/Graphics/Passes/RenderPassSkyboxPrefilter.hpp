#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineSkyboxPrefilter.hpp"
#include "../RenderBufferSkybox.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxPrefilter : public RenderPass {
public:
    explicit RenderPassSkyboxPrefilter(VulkanRenderer& vulkan, RenderBufferSkybox& buffer, RenderResources& resources);

    void setTextureSkybox(const VulkanTexture& value);
    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderPipelineSkyboxPrefilter pipelineSkyboxPrefilter;
    const VulkanTexture* textureSkybox{nullptr};
};
} // namespace Engine
