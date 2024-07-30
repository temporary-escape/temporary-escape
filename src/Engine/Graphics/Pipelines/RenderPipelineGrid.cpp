#include "RenderPipelineGrid.hpp"
#include "../../Assets/AssetsManager.hpp"
#include <component_grid_frag.spirv.h>
#include <component_grid_vert.spirv.h>

using namespace Engine;

RenderPipelineGrid::RenderPipelineGrid(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelineGrid"} {

    addShader(Embed::component_grid_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_grid_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<VoxelShape::VertexFinal>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineGrid::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineGrid::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineGrid::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}
