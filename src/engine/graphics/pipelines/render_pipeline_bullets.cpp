#include "render_pipeline_bullets.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/components/component_turret.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineBullets::RenderPipelineBullets(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineBullets"} {

    addShader(assetsManager.getShaders().find("pass_bullets_vert"));
    addShader(assetsManager.getShaders().find("pass_bullets_frag"));
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
