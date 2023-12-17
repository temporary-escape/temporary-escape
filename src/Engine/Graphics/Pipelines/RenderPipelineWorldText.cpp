#include "RenderPipelineWorldText.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentWorldText.hpp"
#include <component_world_text_frag.spirv.h>
#include <component_world_text_vert.spirv.h>

using namespace Engine;

RenderPipelineWorldText::RenderPipelineWorldText(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineWorldText"} {

    addShader(Embed::component_world_text_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_world_text_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
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
