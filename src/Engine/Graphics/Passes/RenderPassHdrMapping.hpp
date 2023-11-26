#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineHdrMapping.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassHDRMapping : public RenderPass {
public:
    explicit RenderPassHDRMapping(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                  RenderResources& resources, AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    const RenderOptions& options;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineHDRMapping pipelineHDRMapping;
};
} // namespace Engine
