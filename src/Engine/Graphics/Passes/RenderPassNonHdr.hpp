#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineIcons.hpp"
#include "../Pipelines/RenderPipelineLines.hpp"
#include "../Pipelines/RenderPipelineOutline.hpp"
#include "../Pipelines/RenderPipelineWorldText.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassNonHDR : public RenderPass {
public:
    explicit RenderPassNonHDR(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                              RenderResources& resources);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderOutline(VulkanCommandBuffer& vkb, Scene& scene);
    void renderIcons(VulkanCommandBuffer& vkb, Scene& scene);
    void renderWorldText(VulkanCommandBuffer& vkb, Scene& scene);
    // void renderShipControls(VulkanCommandBuffer& vkb, Scene& scene);

    const RenderOptions& options;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineOutline pipelineOutline;
    RenderPipelineIcons pipelineIcons;
    RenderPipelineWorldText pipelineWorldText;
    RenderPipelineLines pipelineLines;
};
} // namespace Engine
