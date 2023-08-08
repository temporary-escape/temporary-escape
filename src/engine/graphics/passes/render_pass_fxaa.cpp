#include "render_pass_fxaa.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassFXAA::RenderPassFXAA(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                               RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassFXAA"},
    buffer{buffer},
    resources{resources},
    pipelineFXAA{vulkan, assetsManager} {

    { // FXAA
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::FXAA, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::FXAA,
        },
        {});

    addPipeline(pipelineFXAA, 0);
}

void RenderPassFXAA::beforeRender(VulkanCommandBuffer& vkb) {
    const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Forward);
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
    barrier.srcAccessMask =
        VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

    vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        barrier);
}

void RenderPassFXAA::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    pipelineFXAA.bind(vkb);

    const auto& texForward = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Forward);

    pipelineFXAA.setTextureSize(Vector2{texForward.getSize2D()});
    pipelineFXAA.flushConstants(vkb);

    pipelineFXAA.setTexture(texForward);
    pipelineFXAA.flushDescriptors(vkb);

    pipelineFXAA.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
