#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineBloomUpsample.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassBloomUpsample : public RenderPass {
public:
    explicit RenderPassBloomUpsample(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                     RenderResources& resources, AssetsManager& assetsManager, uint32_t level);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineBloomUpsample pipelineBloomUpsample;
    const uint32_t level;
};
} // namespace Engine
