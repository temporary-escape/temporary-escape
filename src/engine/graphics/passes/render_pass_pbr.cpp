#include "render_pass_pbr.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/controllers/controller_lights.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassPbr::RenderPassPbr(VulkanRenderer& vulkan, RenderBufferPbr& buffer, RenderResources& resources,
                             AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassPbr"},
    buffer{buffer},
    resources{resources},
    pipelinePbr{vulkan, assetsManager} {

    { // Forward
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::Forward, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::Forward,
        },
        {});

    { // Dependency for Forward
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        addSubpassDependency(dependency);
    }

    addPipeline(pipelinePbr, 0);

    brdf = assetsManager.getTextures().find("brdf");
}

void RenderPassPbr::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;

    const auto transitionRead = [&](const RenderBufferPbr::Attachment attachment) {
        const auto& texture = buffer.getAttachmentTexture(attachment);
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture.getHandle();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture.getMipMaps();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture.getLayerCount();
        barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

        vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            barrier);
    };

    const auto transitionDepth = [&](const RenderBufferPbr::Attachment attachment) {
        const auto& texture = buffer.getAttachmentTexture(attachment);
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture.getHandle();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture.getMipMaps();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture.getLayerCount();
        barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

        vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            barrier);
    };

    transitionRead(RenderBufferPbr::Attachment::AlbedoAmbient);
    transitionRead(RenderBufferPbr::Attachment::EmissiveRoughness);
    transitionRead(RenderBufferPbr::Attachment::SSAO);
    transitionDepth(RenderBufferPbr::Attachment::ShadowL0);
}

void RenderPassPbr::render(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();
    auto& camera = *scene.getPrimaryCamera();

    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    const auto* skybox = scene.getSkybox();
    const auto* skyboxTextures{&resources.getDefaultSkybox()};
    if (skybox) {
        skyboxTextures = &skybox->getTextures();
    }

    pipelinePbr.bind(vkb);

    const auto& texSsao = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::SSAO);
    const auto& texDepth = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Depth);
    const auto& texShadows = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::ShadowL0);
    const auto& texBaseColorAmbient = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::AlbedoAmbient);
    const auto& texEmissiveRoughness = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::EmissiveRoughness);
    const auto& texNormalMetallic = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::NormalMetallic);

    pipelinePbr.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelinePbr.setUniformDirectionalLights(controllerLights.getUboDirectionalLights().getCurrentBuffer());
    pipelinePbr.setUniformShadowsViewProj(controllerLights.getUboShadowsViewProj().getCurrentBuffer());
    pipelinePbr.setTextureIrradiance(skyboxTextures->getIrradiance());
    pipelinePbr.setTexturePrefilter(skyboxTextures->getPrefilter());
    pipelinePbr.setTextureBrdf(brdf->getVulkanTexture());
    pipelinePbr.setTextureDepth(texDepth);
    pipelinePbr.setTextureBaseColorAmbient(texBaseColorAmbient);
    pipelinePbr.setTextureEmissiveRoughness(texEmissiveRoughness);
    pipelinePbr.setTextureNormalMetallic(texNormalMetallic);
    pipelinePbr.setTextureSSAO(texSsao);
    pipelinePbr.setTextureShadows(texShadows);
    pipelinePbr.flushDescriptors(vkb);

    pipelinePbr.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
