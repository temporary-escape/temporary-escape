#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_position_feedback.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

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
