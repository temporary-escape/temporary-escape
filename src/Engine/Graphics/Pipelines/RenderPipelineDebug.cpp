#include "RenderPipelineDebug.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentDebug.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelineDebug::RenderPipelineDebug(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineDebug"} {

    addShader(assetsManager.getShaders().find("component_debug_vert"));
    addShader(assetsManager.getShaders().find("component_debug_frag"));
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
