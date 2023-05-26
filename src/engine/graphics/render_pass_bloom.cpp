#include "render_pass_bloom.hpp"

using namespace Engine;

static const int viewportDivision = 4;

static auto logger = createLogger(LOG_FILENAME);

RenderPassBloom::Downsample::Downsample(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                                        const VulkanTexture& forward) :
    RenderPass{vulkan, viewport / viewportDivision}, forward{forward} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    // Color
    addAttachment({VK_FORMAT_R16G16B16A16_SFLOAT,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void RenderPassBloom::Downsample::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    const auto& dstTexture = getTexture(Attachments::Color);

    std::array<VkImageBlit, 1> blit{};
    blit[0].srcOffsets[0] = {0, 0, 0};
    blit[0].srcOffsets[1] = {
        static_cast<int32_t>(forward.getExtent().width),
        static_cast<int32_t>(forward.getExtent().height),
        1,
    };
    blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].srcSubresource.mipLevel = 0;
    blit[0].srcSubresource.baseArrayLayer = 0;
    blit[0].srcSubresource.layerCount = 1;
    blit[0].dstOffsets[0] = {0, 0, 0};
    blit[0].dstOffsets[1] = {
        static_cast<int32_t>(dstTexture.getExtent().width),
        static_cast<int32_t>(dstTexture.getExtent().height),
        1,
    };
    blit[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].dstSubresource.mipLevel = 0;
    blit[0].dstSubresource.baseArrayLayer = 0;
    blit[0].dstSubresource.layerCount = 1;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = dstTexture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = dstTexture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

    vkb.blitImage(forward,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  dstTexture,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  blit,
                  VK_FILTER_LINEAR);

    barrier = VkImageMemoryBarrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = dstTexture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = dstTexture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

    barrier = VkImageMemoryBarrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = forward.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = forward.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
}

RenderPassBloom::Extract::Extract(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                                  const VulkanTexture& forward) :
    RenderPass{vulkan, viewport / viewportDivision}, subpassBloomExtract{vulkan, assetsManager, forward} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    // Color
    addAttachment({VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    addSubpass(subpassBloomExtract);
    init();
}

void RenderPassBloom::Extract::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = this->viewport;

    renderPassInfo.clearValues.resize(1);
    renderPassInfo.clearValues[Attachments::Color].color = {{1.0f, 1.0f, 1.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, this->viewport);
    vkb.setScissor({0, 0}, this->viewport);

    subpassBloomExtract.render(vkb);

    vkb.endRenderPass();
}

RenderPassBloom::Blur::Blur(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                            const VulkanTexture& dst, const VulkanTexture& color, const bool vertical) :
    RenderPass{vulkan, viewport / viewportDivision}, subpassBloomBlur{vulkan, assetsManager, color, vertical} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addAttachment(
        dst, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addSubpass(subpassBloomBlur);
    init();
}

void RenderPassBloom::Blur::reset() {
    subpassBloomBlur.reset();
}

void RenderPassBloom::Blur::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = this->viewport;

    renderPassInfo.clearValues.resize(1);
    renderPassInfo.clearValues[Attachments::Color].color = {{1.0f, 1.0f, 1.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, this->viewport);
    vkb.setScissor({0, 0}, this->viewport);

    subpassBloomBlur.render(vkb);

    vkb.endRenderPass();
}

RenderPassBloom::RenderPassBloom(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                                 const VulkanTexture& forward) :
    downsample{vulkan, assetsManager, viewport, forward},
    extract{vulkan, assetsManager, viewport, downsample.getTexture(Downsample::Attachments::Color)},
    blurH{vulkan,
          assetsManager,
          viewport,
          downsample.getTexture(Downsample::Attachments::Color),
          extract.getTexture(Downsample::Attachments::Color),
          false},
    blurV{vulkan,
          assetsManager,
          viewport,
          extract.getTexture(Downsample::Attachments::Color),
          downsample.getTexture(Downsample::Attachments::Color),
          true} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);
}

void RenderPassBloom::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    downsample.render(vkb, viewport, scene);
    extract.render(vkb, viewport, scene);
    blurH.reset();
    blurV.reset();
    for (auto i = 0; i < 3; i++) {
        blurH.render(vkb, viewport, scene);
        blurV.render(vkb, viewport, scene);
    }
}
