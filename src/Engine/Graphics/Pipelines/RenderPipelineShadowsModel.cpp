#include "RenderPipelineShadowsModel.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Camera.hpp"
#include "../../Scene/Components/ComponentModel.hpp"
#include <component_model_vert.spirv.h>
#include <component_shadow_frag.spirv.h>

using namespace Engine;

RenderPipelineShadowsModel::RenderPipelineShadowsModel(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineShadowsModel"} {

    addShader(Embed::component_model_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_shadow_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
    setDepthClamp(DepthClamp::Enabled);
    setDynamic("Camera");
}

void RenderPipelineShadowsModel::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineShadowsModel::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineShadowsModel::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}
