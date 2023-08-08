#include "render_pass_hdr_mapping.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassHDRMapping::RenderPassHDRMapping(VulkanRenderer& vulkan, RenderBufferPbr& buffer, RenderResources& resources,
                                           AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassHDRMapping"},
    buffer{buffer},
    resources{resources},
    pipelineHDRMapping{vulkan, assetsManager} {

    { // FXAA
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::Forward, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::Forward,
        },
        {});

    addPipeline(pipelineHDRMapping, 0);
}

void RenderPassHDRMapping::beforeRender(VulkanCommandBuffer& vkb) {
    { // Bloom color
        const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::BloomL0);
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

    { // FXAA
        const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::FXAA);
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

void RenderPassHDRMapping::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    pipelineHDRMapping.bind(vkb);

    const auto& texColor = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::FXAA);
    const auto& texBloom = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::BloomL0);

    pipelineHDRMapping.setBloomStrength(0.08f);
    pipelineHDRMapping.setExposure(1.0f);
    pipelineHDRMapping.setGamma(2.2f);
    pipelineHDRMapping.flushConstants(vkb);
    pipelineHDRMapping.setTextureColor(texColor);
    pipelineHDRMapping.setTextureBloom(texBloom);
    pipelineHDRMapping.flushDescriptors(vkb);

    pipelineHDRMapping.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
