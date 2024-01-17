#include "RenderPipelineSpaceDust.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <pass_space_dust_frag.spirv.h>
#include <pass_space_dust_vert.spirv.h>

using namespace Engine;

RenderPipelineSpaceDust::RenderPipelineSpaceDust(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineSpaceDust"} {

    addShader(Embed::pass_space_dust_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_space_dust_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<SpaceDustVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineSpaceDust::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineSpaceDust::setMoveDirection(const Vector3& value) {
    pushConstants(PushConstant{"moveDirection", value});
}

void RenderPipelineSpaceDust::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineSpaceDust::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineSpaceDust::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
