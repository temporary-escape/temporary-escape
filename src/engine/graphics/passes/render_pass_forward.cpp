#include "render_pass_forward.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassForward::RenderPassForward(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                     RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassForward"}, buffer{buffer}, resources{resources} {

    { // Depth
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::Depth, attachment);
    }

    { // Forward
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::Forward, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::Depth,
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

    { // Dependency for Depth
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        addSubpassDependency(dependency);
    }
}

void RenderPassForward::beforeRender(VulkanCommandBuffer& vkb) {
    /*const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Depth);
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
    barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        barrier);*/
}

void RenderPassForward::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());
}
