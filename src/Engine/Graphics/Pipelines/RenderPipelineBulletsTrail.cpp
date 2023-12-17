#include "RenderPipelineBulletsTrail.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentTurret.hpp"
#include <pass_bullets_trail_frag.spirv.h>
#include <pass_bullets_trail_vert.spirv.h>

using namespace Engine;

RenderPipelineBulletsTrail::RenderPipelineBulletsTrail(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineBulletsTrail"} {

    addShader(Embed::pass_bullets_trail_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_bullets_trail_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentTurret::BulletInstance>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_LINE);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Additive);
    if (vulkan.getPhysicalDeviceFeatures().wideLines) {
        setLineWidth(2.0f);
    }
}

void RenderPipelineBulletsTrail::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineBulletsTrail::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
