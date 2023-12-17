#include "RenderPassSkyboxIrradiance.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassSkyboxIrradiance::RenderPassSkyboxIrradiance(VulkanRenderer& vulkan, RenderBufferSkybox& buffer,
                                                       RenderResources& resources) :
    RenderPass{vulkan, buffer, "RenderPassSkyboxIrradiance"},
    vulkan{vulkan},
    resources{resources},
    pipelineSkyboxIrradiance{vulkan} {

    { // Color
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};
        addAttachment(RenderBufferSkybox::Attachment::Irradiance, attachment);
    }

    addSubpass(
        {
            RenderBufferSkybox::Attachment::Irradiance,
        },
        {});

    addPipeline(pipelineSkyboxIrradiance, 0);
}

void RenderPassSkyboxIrradiance::setTextureSkybox(const VulkanTexture& value) {
    textureSkybox = &value;
}

void RenderPassSkyboxIrradiance::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassSkyboxIrradiance::render(VulkanCommandBuffer& vkb, Scene& scene) {
    const auto& camera = *scene.getPrimaryCamera();

    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    const Vector2i viewport{getViewport().y, getViewport().y};

    for (uint32_t i = 0; i < getMipMapLevels(viewport); i++) {
        const auto offset = mipMapOffset(viewport, i);
        const auto size = mipMapSize(viewport, i);

        vkb.setViewport(offset, size);
        vkb.setScissor(offset, size);

        pipelineSkyboxIrradiance.bind(vkb);

        pipelineSkyboxIrradiance.setProjectionViewMatrix(camera.getProjectionMatrix() * camera.getViewMatrix());
        pipelineSkyboxIrradiance.flushConstants(vkb);

        pipelineSkyboxIrradiance.setTextureSkybox(*textureSkybox);
        pipelineSkyboxIrradiance.flushDescriptors(vkb);

        pipelineSkyboxIrradiance.renderMesh(vkb, resources.getMeshSkyboxCube());
    }
}
