#include "RenderPipelineSkyboxIrradiance.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <skybox_irradiance_frag.spirv.h>
#include <skybox_irradiance_vert.spirv.h>

using namespace Engine;

RenderPipelineSkyboxIrradiance::RenderPipelineSkyboxIrradiance(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineSkyboxIrradiance"} {

    addShader(Embed::skybox_irradiance_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::skybox_irradiance_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<SkyboxVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineSkyboxIrradiance::setProjectionViewMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"projectionViewMatrix", value});
}

void RenderPipelineSkyboxIrradiance::setTextureSkybox(const VulkanTexture& texture) {
    textures[0] = {"texSkybox", texture};
}

void RenderPipelineSkyboxIrradiance::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
