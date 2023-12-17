#include "RenderPipelinePositionFeedback.hpp"
#include "../../Assets/AssetsManager.hpp"
#include <position_feedback_comp.spirv.h>

using namespace Engine;

RenderPipelinePositionFeedback::RenderPipelinePositionFeedback(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelinePositionFeedback"} {

    addShader(Embed::position_feedback_comp_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
    setCompute(true);
}

void RenderPipelinePositionFeedback::setViewport(const Vector2& value) {
    pushConstants(PushConstant{"viewport", value});
}

void RenderPipelinePositionFeedback::setCount(const int value) {
    pushConstants(PushConstant{"count", value});
}

void RenderPipelinePositionFeedback::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelinePositionFeedback::setBufferInput(const VulkanBuffer& ubo) {
    uniforms[1] = {"InputBuffer", ubo};
}

void RenderPipelinePositionFeedback::setBufferOutput(const VulkanBuffer& ubo) {
    uniforms[2] = {"OutputBuffer", ubo};
}

void RenderPipelinePositionFeedback::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
