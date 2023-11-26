#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineSsao.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassSSAO : public RenderPass {
public:
    explicit RenderPassSSAO(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                            RenderResources& resources, AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    const RenderOptions& options;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineSSAO pipelineSSAO;
};
} // namespace Engine
