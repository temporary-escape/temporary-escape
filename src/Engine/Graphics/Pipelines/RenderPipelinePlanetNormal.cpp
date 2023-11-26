#include "RenderPipelinePlanetNormal.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelinePlanetNormal::RenderPipelinePlanetNormal(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelinePlanetColor"} {

    addShader(assetsManager.getShaders().find("planet_normal_vert"));
    addShader(assetsManager.getShaders().find("planet_normal_frag"));
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
