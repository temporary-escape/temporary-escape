#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineBloomDownsample.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassBloomDownsample : public RenderPass {
public:
    explicit RenderPassBloomDownsample(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                       RenderResources& resources, AssetsManager& assetsManager, uint32_t level);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    const RenderOptions& options;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineBloomDownsample pipelineBloomDownsample;
    const uint32_t level;
};
} // namespace Engine
