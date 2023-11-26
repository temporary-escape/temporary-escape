#include "RenderPipelinePolyShape.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentPolyShape.hpp"

using namespace Engine;

RenderPipelinePolyShape::RenderPipelinePolyShape(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelinePolyShape"} {

    addShader(assetsManager.getShaders().find("component_poly_shape_vert"));
    addShader(assetsManager.getShaders().find("component_poly_shape_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentPolyShape::Point>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelinePolyShape::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelinePolyShape::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelinePolyShape::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
