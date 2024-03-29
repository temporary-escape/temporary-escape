#include "RenderPipelineShadowsGrid.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Camera.hpp"
#include <component_grid_vert.spirv.h>
#include <component_shadow_frag.spirv.h>

using namespace Engine;

RenderPipelineShadowsGrid::RenderPipelineShadowsGrid(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineShadowsGrid"} {

    addShader(Embed::component_grid_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_shadow_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<VoxelShape::VertexFinal>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
    setDepthClamp(DepthClamp::Enabled);
}

void RenderPipelineShadowsGrid::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineShadowsGrid::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineShadowsGrid::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}

void RenderPipelineShadowsGrid::setUniformCamera(const VulkanBuffer& ubo, const uint32_t index) {
    uniforms[0] = {"Camera", ubo, index * sizeof(Camera::Uniform), sizeof(Camera::Uniform)};
}

void RenderPipelineShadowsGrid::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
