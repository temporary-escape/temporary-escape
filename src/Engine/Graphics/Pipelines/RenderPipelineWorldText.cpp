#include "RenderPipelineWorldText.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentWorldText.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelineWorldText::RenderPipelineWorldText(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineWorldText"} {

    addShader(assetsManager.getShaders().find("component_world_text_vert"));
    addShader(assetsManager.getShaders().find("component_world_text_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentWorldText::Vertex>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineWorldText::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineWorldText::setColor(const Color4& value) {
    pushConstants(PushConstant{"color", value});
}

void RenderPipelineWorldText::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineWorldText::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineWorldText::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
