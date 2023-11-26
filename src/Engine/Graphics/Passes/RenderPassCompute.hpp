#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelinePositionFeedback.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassCompute : public RenderPass {
public:
    explicit RenderPassCompute(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                               AssetsManager& assetsManager);

    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderPositionFeedback(VulkanCommandBuffer& vkb, Scene& scene);

    RenderPipelinePositionFeedback pipelinePositionFeedback;
};
} // namespace Engine
