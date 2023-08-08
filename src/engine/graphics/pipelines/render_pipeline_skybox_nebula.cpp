#include "render_pipeline_skybox_nebula.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineSkyboxNebula::RenderPipelineSkyboxNebula(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineSkyboxNebula"} {

    addShader(assetsManager.getShaders().find("skybox_nebula_vert"));
    addShader(assetsManager.getShaders().find("skybox_nebula_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<SkyboxVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Additive);
}

void RenderPipelineSkyboxNebula::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineSkyboxNebula::setUniformNebula(const VulkanBuffer& ubo) {
    uniforms[1] = {"Nebula", ubo};
}

void RenderPipelineSkyboxNebula::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
