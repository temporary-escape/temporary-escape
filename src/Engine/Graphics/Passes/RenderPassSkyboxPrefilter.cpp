#include "RenderPassSkyboxPrefilter.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassSkyboxPrefilter::RenderPassSkyboxPrefilter(VulkanRenderer& vulkan, RenderBufferSkybox& buffer,
                                                     RenderResources& resources) :
    RenderPass{vulkan, buffer, "RenderPassSkyboxPrefilter"},
    vulkan{vulkan},
    resources{resources},
    pipelineSkyboxPrefilter{vulkan} {

    { // Color
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};
        addAttachment(RenderBufferSkybox::Attachment::Prefilter, attachment);
    }

    addSubpass(
        {
            RenderBufferSkybox::Attachment::Prefilter,
        },
        {});

    addPipeline(pipelineSkyboxPrefilter, 0);
}

void RenderPassSkyboxPrefilter::setTextureSkybox(const VulkanTexture& value) {
    textureSkybox = &value;
}

void RenderPassSkyboxPrefilter::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassSkyboxPrefilter::render(VulkanCommandBuffer& vkb, Scene& scene) {
    const auto& camera = *scene.getPrimaryCamera();

    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    const Vector2i viewport{getViewport().y, getViewport().y};

    for (uint32_t i = 0; i < getMipMapLevels(viewport); i++) {
        const auto offset = mipMapOffset(viewport, i);
        const auto size = mipMapSize(viewport, i);
        const auto roughness = static_cast<float>(i) / static_cast<float>((getMipMapLevels(viewport) - 1));

        vkb.setViewport(offset, size);
        vkb.setScissor(offset, size);

        pipelineSkyboxPrefilter.bind(vkb);

        pipelineSkyboxPrefilter.setProjectionViewMatrix(camera.getProjectionMatrix() * camera.getViewMatrix());
        pipelineSkyboxPrefilter.setRoughness(roughness);
        pipelineSkyboxPrefilter.flushConstants(vkb);

        pipelineSkyboxPrefilter.setTextureSkybox(*textureSkybox);
        pipelineSkyboxPrefilter.flushDescriptors(vkb);

        pipelineSkyboxPrefilter.renderMesh(vkb, resources.getMeshSkyboxCube());
    }
}
