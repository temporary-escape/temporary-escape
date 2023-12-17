#include "RenderPipelinePlanetNormal.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <planet_normal_frag.spirv.h>
#include <planet_normal_vert.spirv.h>

using namespace Engine;

RenderPipelinePlanetNormal::RenderPipelinePlanetNormal(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelinePlanetColor"} {

    addShader(Embed::planet_normal_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::planet_normal_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelinePlanetNormal::setTextureHeight(const VulkanTexture& texture) {
    textures[0] = {"textureHeightmap", texture};
}

void RenderPipelinePlanetNormal::setResolution(float value) {
    pushConstants(PushConstant{"resolution", value});
}

void RenderPipelinePlanetNormal::setWaterLevel(float value) {
    pushConstants(PushConstant{"waterLevel", value});
}

void RenderPipelinePlanetNormal::setStrength(float value) {
    pushConstants(PushConstant{"strength", value});
}

void RenderPipelinePlanetNormal::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
