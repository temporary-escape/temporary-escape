#include "RenderPipelinePolyShape.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentPolyShape.hpp"
#include <component_poly_shape_frag.spirv.h>
#include <component_poly_shape_vert.spirv.h>

using namespace Engine;

RenderPipelinePolyShape::RenderPipelinePolyShape(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelinePolyShape"} {

    addShader(Embed::component_poly_shape_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_poly_shape_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
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
