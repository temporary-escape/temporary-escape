#include "RenderPipelineIcons.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentIcon.hpp"

using namespace Engine;

RenderPipelineIcons::RenderPipelineIcons(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineIcons"} {

    addShader(assetsManager.getShaders().find("component_icons_vert"));
    addShader(assetsManager.getShaders().find("component_icons_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentIcon::Point>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineIcons::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineIcons::setScale(float scale) {
    pushConstants(PushConstant{"scale", scale});
}

void RenderPipelineIcons::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineIcons::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineIcons::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
