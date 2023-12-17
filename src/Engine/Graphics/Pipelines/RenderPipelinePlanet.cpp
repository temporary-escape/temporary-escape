#include "RenderPipelinePlanet.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <pass_skybox_planet_frag.spirv.h>
#include <pass_skybox_planet_vert.spirv.h>

using namespace Engine;

RenderPipelinePlanet::RenderPipelinePlanet(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelinePlanet"} {

    addShader(Embed::pass_skybox_planet_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_skybox_planet_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<PlanetVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setStencil(Stencil::Write, 0xff);
    setBlending(Blending::Normal);
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
    uniforms[2] = {"Atmosphere", ubo};
}

void RenderPipelinePlanet::setUniformDirectionalLights(const VulkanBuffer& ubo) {
    uniforms[1] = {"DirectionalLights", ubo};
}

void RenderPipelinePlanet::setTextureAlbedo(const VulkanTexture& texture) {
    textures[0] = {"albedoTexture", texture};
}

void RenderPipelinePlanet::setTextureNormal(const VulkanTexture& texture) {
    textures[1] = {"normalTexture", texture};
}

void RenderPipelinePlanet::setTextureMetallicRoughness(const VulkanTexture& texture) {
    textures[2] = {"metallicRoughnessTexture", texture};
}

void RenderPipelinePlanet::setTextureSkyboxIrradiance(const VulkanTexture& texture) {
    textures[3] = {"irradianceTexture", texture};
}

void RenderPipelinePlanet::setTextureSkyboxPrefilter(const VulkanTexture& texture) {
    textures[4] = {"prefilterTexture", texture};
}

void RenderPipelinePlanet::setTextureBrdf(const VulkanTexture& texture) {
    textures[5] = {"brdfTexture", texture};
}

void RenderPipelinePlanet::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
