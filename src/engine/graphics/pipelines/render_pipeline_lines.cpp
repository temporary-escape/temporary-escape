#include "render_pipeline_lines.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/components/component_lines.hpp"

using namespace Engine;

RenderPipelineLines::RenderPipelineLines(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineLines"} {

    addShader(assetsManager.getShaders().find("component_lines_vert"));
    addShader(assetsManager.getShaders().find("component_lines_frag"));
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
