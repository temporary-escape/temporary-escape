#include "RenderPipelineBullets.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentTurret.hpp"
#include "../MeshUtils.hpp"
#include <pass_bullets_frag.spirv.h>
#include <pass_bullets_vert.spirv.h>

using namespace Engine;

RenderPipelineBullets::RenderPipelineBullets(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelineBullets"} {

    addShader(Embed::pass_bullets_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_bullets_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<BulletVertex>(0));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentTurret::BulletInstance>(1, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Additive);
}

void RenderPipelineBullets::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineBullets::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
