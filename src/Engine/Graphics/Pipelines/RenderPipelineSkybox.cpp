#include "RenderPipelineSkybox.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelineSkybox::RenderPipelineSkybox(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineSkybox"} {

    addShader(assetsManager.getShaders().find("pass_skybox_vert"));
    addShader(assetsManager.getShaders().find("pass_skybox_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<SkyboxVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineSkybox::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineSkybox::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineSkybox::setTextureBaseColor(const VulkanTexture& texture) {
    textures[0] = {"texBaseColor", texture};
}

void RenderPipelineSkybox::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
