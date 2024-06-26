#include "RenderPipelineModelInstanced.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentModel.hpp"
#include <component_model_frag.spirv.h>
#include <component_model_instanced_vert.spirv.h>

using namespace Engine;

RenderPipelineModelInstanced::RenderPipelineModelInstanced(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineModel"} {

    addShader(Embed::component_model_instanced_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_model_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::InstancedVertex>(
        1, VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineModelInstanced::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineModelInstanced::setUniformMaterial(const VulkanBuffer& ubo) {
    uniforms[1] = {"Material", ubo};
}

void RenderPipelineModelInstanced::setTextureBaseColor(const VulkanTexture& texture) {
    textures[0] = {"baseColorTexture", texture};
}

void RenderPipelineModelInstanced::setTextureEmissive(const VulkanTexture& texture) {
    textures[1] = {"emissiveTexture", texture};
}

void RenderPipelineModelInstanced::setTextureMetallicRoughness(const VulkanTexture& texture) {
    textures[2] = {"metallicRoughnessTexture", texture};
}

void RenderPipelineModelInstanced::setTextureNormal(const VulkanTexture& texture) {
    textures[3] = {"normalTexture", texture};
}

void RenderPipelineModelInstanced::setTextureAmbientOcclusion(const VulkanTexture& texture) {
    textures[4] = {"ambientOcclusionTexture", texture};
}

void RenderPipelineModelInstanced::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
