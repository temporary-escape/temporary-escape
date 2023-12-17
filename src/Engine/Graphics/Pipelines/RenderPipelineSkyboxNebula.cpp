#include "RenderPipelineSkyboxNebula.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <skybox_nebula_frag.spirv.h>
#include <skybox_nebula_vert.spirv.h>

using namespace Engine;

RenderPipelineSkyboxNebula::RenderPipelineSkyboxNebula(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineSkyboxNebula"} {

    addShader(Embed::skybox_nebula_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::skybox_nebula_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
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
