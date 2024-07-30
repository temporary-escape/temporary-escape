#include "RenderPipelineShadowsModelSkinned.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Camera.hpp"
#include "../../Scene/Components/ComponentModelSkinned.hpp"
#include <component_shadow_frag.spirv.h>
#include <component_shadow_model_skinned_vert.spirv.h>

using namespace Engine;

RenderPipelineShadowsModelSkinned::RenderPipelineShadowsModelSkinned(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineShadowsModelSkinned"} {

    addShader(Embed::component_shadow_model_skinned_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_shadow_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModelSkinned::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
    setDepthClamp(DepthClamp::Enabled);
    setDynamic("Camera");
    setDynamic("Armature");
}

void RenderPipelineShadowsModelSkinned::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineShadowsModelSkinned::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineShadowsModelSkinned::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}
