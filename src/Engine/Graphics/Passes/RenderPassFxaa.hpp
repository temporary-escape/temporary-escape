#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineFxaa.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassFXAA : public RenderPass {
public:
    explicit RenderPassFXAA(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                            RenderResources& resources, AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineFXAA pipelineFXAA;
};
} // namespace Engine
