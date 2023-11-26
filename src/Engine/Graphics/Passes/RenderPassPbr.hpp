#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelinePbr.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassPbr : public RenderPass {
public:
    explicit RenderPassPbr(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                           RenderResources& resources, AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    const RenderOptions& options;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelinePbr pipelinePbr;
    TexturePtr brdf;
};
} // namespace Engine
