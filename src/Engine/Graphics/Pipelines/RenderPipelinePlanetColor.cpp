#include "RenderPipelinePlanetColor.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <planet_color_frag.spirv.h>
#include <planet_color_vert.spirv.h>

using namespace Engine;

RenderPipelinePlanetColor::RenderPipelinePlanetColor(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelinePlanetColor"} {

    addShader(Embed::planet_color_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::planet_color_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelinePlanetColor::setTextureBiome(const VulkanTexture& texture) {
    textures[0] = {"textureBiome", texture};
}

void RenderPipelinePlanetColor::setTextureRoughness(const VulkanTexture& texture) {
    textures[1] = {"textureRoughness", texture};
}

void RenderPipelinePlanetColor::setInputHeightmap(const VulkanTexture& texture) {
    inputs[0] = {"samplerHeightmap", texture};
}

void RenderPipelinePlanetColor::setInputMoisture(const VulkanTexture& texture) {
    inputs[1] = {"samplerMoisture", texture};
}

void RenderPipelinePlanetColor::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, inputs);
}
