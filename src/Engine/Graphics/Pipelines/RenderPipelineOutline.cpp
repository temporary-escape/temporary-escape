#include "RenderPipelineOutline.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelineOutline::RenderPipelineOutline(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineOutline"} {

    addShader(assetsManager.getShaders().find("pass_outline_vert"));
    addShader(assetsManager.getShaders().find("pass_outline_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::Additive);
}

void RenderPipelineOutline::setColorSelected(const Color4& value) {
    pushConstants(PushConstant{"selectedColor", value});
}
void RenderPipelineOutline::setColorFinal(const Color4& value) {
    pushConstants(PushConstant{"finalColor", value});
}
void RenderPipelineOutline::setThickness(float value) {
    pushConstants(PushConstant{"thickness", value});
}
void RenderPipelineOutline::setTextureEntity(const VulkanTexture& texture) {
    textures[0] = {"entityColorTexture", texture};
}

void RenderPipelineOutline::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
