#include "RenderPipelineLines.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentLines.hpp"
#include <component_lines_frag.spirv.h>
#include <component_lines_vert.spirv.h>

using namespace Engine;

RenderPipelineLines::RenderPipelineLines(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelineLines"} {

    addShader(Embed::component_lines_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_lines_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentLines::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineLines::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineLines::setColor(const Color4& value) {
    pushConstants(PushConstant{"color", value});
}

void RenderPipelineLines::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineLines::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
