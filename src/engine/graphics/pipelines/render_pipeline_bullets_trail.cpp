#include "render_pipeline_bullets_trail.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/components/component_turret.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineBulletsTrail::RenderPipelineBulletsTrail(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineBulletsTrail"} {

    addShader(assetsManager.getShaders().find("pass_bullets_trail_vert"));
    addShader(assetsManager.getShaders().find("pass_bullets_trail_frag"));
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
