#include "RenderPipelineTacticalOverlaySpots.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentIcon.hpp"
#include <pass_tactical_overview_spots_frag.spirv.h>
#include <pass_tactical_overview_spots_vert.spirv.h>

using namespace Engine;

RenderPipelineTacticalOverlaySpots::RenderPipelineTacticalOverlaySpots(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineTacticalOverlaySpots"} {

    addShader(Embed::pass_tactical_overview_spots_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_tactical_overview_spots_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentIcon::Point>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_NONE);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineTacticalOverlaySpots::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineTacticalOverlaySpots::setColor(const Color4& value) {
    pushConstants(PushConstant{"color", value});
}

void RenderPipelineTacticalOverlaySpots::setPlayerPos(const Vector3& value) {
    pushConstants(PushConstant{"playerPos", value});
}

void RenderPipelineTacticalOverlaySpots::setScale(float value) {
    pushConstants(PushConstant{"scale", value});
}

void RenderPipelineTacticalOverlaySpots::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineTacticalOverlaySpots::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
