#include "RenderPassBloomDownsample.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassBloomDownsample::RenderPassBloomDownsample(const RenderOptions& options, VulkanRenderer& vulkan,
                                                     RenderBufferPbr& buffer, RenderResources& resources,
                                                     const uint32_t level) :
    RenderPass{vulkan, buffer, "RenderPassBloomDownsample"},
    options{options},
    buffer{buffer},
    resources{resources},
    pipelineBloomDownsample{vulkan},
    level{level} {

    { // Bloom
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::BloomL0 + level, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::BloomL0 + level,
        },
        {});

    addPipeline(pipelineBloomDownsample, 0);
}

void RenderPassBloomDownsample::beforeRender(VulkanCommandBuffer& vkb) {
    if (level > 0) {
        const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::BloomL0 + level - 1);
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
    } else {
        if (!options.fxaa) { // FXAA
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
            barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

            vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                barrier);
        }
    }
}

void RenderPassBloomDownsample::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    const auto attachment =
        level == 0 ? RenderBufferPbr::Attachment::Forward : RenderBufferPbr::Attachment::BloomL0 + level - 1;
    const auto& texture = buffer.getAttachmentTexture(attachment);

    pipelineBloomDownsample.bind(vkb);
    pipelineBloomDownsample.setTextureSize(texture.getSize2D());
    pipelineBloomDownsample.setMipLevel(static_cast<int>(level));
    pipelineBloomDownsample.flushConstants(vkb);
    pipelineBloomDownsample.setTexture(texture);
    pipelineBloomDownsample.flushDescriptors(vkb);

    pipelineBloomDownsample.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
