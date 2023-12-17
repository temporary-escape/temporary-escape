#include "RenderPassBloomUpsample.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassBloomUpsample::RenderPassBloomUpsample(const RenderOptions& options, VulkanRenderer& vulkan,
                                                 RenderBufferPbr& buffer, RenderResources& resources,
                                                 const uint32_t level) :
    RenderPass{vulkan, buffer, "RenderPassBloomUpsample"},
    buffer{buffer},
    resources{resources},
    pipelineBloomUpsample{vulkan},
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

    addPipeline(pipelineBloomUpsample, 0);
}

void RenderPassBloomUpsample::beforeRender(VulkanCommandBuffer& vkb) {
    const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::BloomL0 + level + 1);
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

void RenderPassBloomUpsample::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::BloomL0 + level + 1);

    const float filterRadius = 0.005f;
    pipelineBloomUpsample.bind(vkb);
    pipelineBloomUpsample.setFilterRadius(filterRadius);
    pipelineBloomUpsample.flushConstants(vkb);
    pipelineBloomUpsample.setTexture(texture);
    pipelineBloomUpsample.flushDescriptors(vkb);

    pipelineBloomUpsample.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
