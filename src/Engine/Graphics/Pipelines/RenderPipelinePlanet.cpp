#include "RenderPipelinePlanet.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentModel.hpp"
#include <component_planet_frag.spirv.h>
#include <component_planet_vert.spirv.h>

using namespace Engine;

RenderPipelinePlanet::RenderPipelinePlanet(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelinePlanet"} {
    addShader(Embed::component_planet_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_planet_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelinePlanet::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelinePlanet::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelinePlanet::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelinePlanet::setUniformAtmosphere(const VulkanBuffer& ubo) {
    uniforms[1] = {"Atmosphere", ubo};
}

void RenderPipelinePlanet::setTextureBaseColor(const VulkanTexture& texture) {
    textures[0] = {"baseColorTexture", texture};
}

void RenderPipelinePlanet::setTextureMetallicRoughness(const VulkanTexture& texture) {
    textures[1] = {"metallicRoughnessTexture", texture};
}

void RenderPipelinePlanet::setTextureNormal(const VulkanTexture& texture) {
    textures[2] = {"normalTexture", texture};
}

void RenderPipelinePlanet::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
