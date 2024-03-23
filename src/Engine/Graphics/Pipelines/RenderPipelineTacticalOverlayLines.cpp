#include "RenderPipelineTacticalOverlayLines.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentIcon.hpp"
#include <pass_tactical_overview_lines_frag.spirv.h>
#include <pass_tactical_overview_lines_vert.spirv.h>

using namespace Engine;

RenderPipelineTacticalOverlayLines::RenderPipelineTacticalOverlayLines(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineTacticalOverlayLines"} {

    addShader(Embed::pass_tactical_overview_lines_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_tactical_overview_lines_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentIcon::Point>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineTacticalOverlayLines::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineTacticalOverlayLines::setColor(const Color4& value) {
    pushConstants(PushConstant{"color", value});
}

void RenderPipelineTacticalOverlayLines::setPlayerPos(const Vector3& value) {
    pushConstants(PushConstant{"playerPos", value});
}

void RenderPipelineTacticalOverlayLines::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineTacticalOverlayLines::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
