#include "RenderPipelineModel.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentModel.hpp"
#include <component_model_frag.spirv.h>
#include <component_model_vert.spirv.h>

using namespace Engine;

RenderPipelineModel::RenderPipelineModel(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelineModel"} {

    addShader(Embed::component_model_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_model_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineModel::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineModel::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineModel::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}

void RenderPipelineModel::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineModel::setUniformMaterial(const VulkanBuffer& ubo) {
    uniforms[1] = {"Material", ubo};
}

void RenderPipelineModel::setTextureBaseColor(const VulkanTexture& texture) {
    textures[0] = {"baseColorTexture", texture};
}

void RenderPipelineModel::setTextureEmissive(const VulkanTexture& texture) {
    textures[1] = {"emissiveTexture", texture};
}

void RenderPipelineModel::setTextureMetallicRoughness(const VulkanTexture& texture) {
    textures[2] = {"metallicRoughnessTexture", texture};
}

void RenderPipelineModel::setTextureNormal(const VulkanTexture& texture) {
    textures[3] = {"normalTexture", texture};
}

void RenderPipelineModel::setTextureAmbientOcclusion(const VulkanTexture& texture) {
    textures[4] = {"ambientOcclusionTexture", texture};
}

void RenderPipelineModel::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
