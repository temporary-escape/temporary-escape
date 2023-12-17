#include "RenderPipelineDebug.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentDebug.hpp"
#include "../MeshUtils.hpp"
#include <component_debug_frag.spirv.h>
#include <component_debug_vert.spirv.h>

using namespace Engine;

RenderPipelineDebug::RenderPipelineDebug(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelineDebug"} {

    addShader(Embed::component_debug_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_debug_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<BulletVertex>(0));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentDebug::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_LINE);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineDebug::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineDebug::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineDebug::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
