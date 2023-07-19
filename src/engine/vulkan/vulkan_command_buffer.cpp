#include "vulkan_command_buffer.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_command_pool.hpp"
#include "vulkan_descriptor_pool.hpp"
#include "vulkan_descriptor_set.hpp"
#include "vulkan_descriptor_set_layout.hpp"
#include "vulkan_device.hpp"
#include "vulkan_framebuffer.hpp"
#include "vulkan_pipeline.hpp"
#include "vulkan_query_pool.hpp"

using namespace Engine;

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, VulkanCommandPool& commandPool,
                                         VulkanDescriptorPool& descriptorPool) :
    device{device.getDevice()}, commandPool{commandPool.getHandle()}, descriptorPool{&descriptorPool} {

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool.getHandle();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate command buffer!");
    }
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    destroy();
}

VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept {
    swap(other);
}

VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanCommandBuffer::swap(VulkanCommandBuffer& other) noexcept {
    std::swap(device, other.device);
    std::swap(commandPool, other.commandPool);
    std::swap(commandBuffer, other.commandBuffer);
    std::swap(descriptorPool, other.descriptorPool);
}

void VulkanCommandBuffer::destroy() {
    if (commandBuffer) {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        commandBuffer = VK_NULL_HANDLE;
    }
}

void VulkanCommandBuffer::start(const VkCommandBufferBeginInfo& beginInfo) {
    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        EXCEPTION("Failed to begin recording command buffer!");
    }
}

void VulkanCommandBuffer::end() {
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to record command buffer!");
    }
}

void VulkanCommandBuffer::beginRenderPass(const VulkanRenderPassBeginInfo& renderPassInfo) {
    VkRenderPassBeginInfo info{};

    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = renderPassInfo.renderPass->getHandle();
    info.framebuffer = renderPassInfo.framebuffer->getHandle();
    info.renderArea.offset = {renderPassInfo.offset.x, renderPassInfo.offset.y};
    info.renderArea.extent =
        VkExtent2D{static_cast<uint32_t>(renderPassInfo.size.x), static_cast<uint32_t>(renderPassInfo.size.y)};

    info.clearValueCount = static_cast<uint32_t>(renderPassInfo.clearValues.size());
    info.pClearValues = renderPassInfo.clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::nextSubpass() {
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::endRenderPass() {
    vkCmdEndRenderPass(commandBuffer);
}

void VulkanCommandBuffer::setViewport(const Vector2i& pos, const Vector2i& size, const float minDepth,
                                      const float maxDepth) {
    VkViewport viewport{};
    viewport.x = static_cast<float>(pos.x);
    viewport.y = static_cast<float>(pos.y);
    viewport.width = static_cast<float>(size.x);
    viewport.height = static_cast<float>(size.y);
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::setScissor(const Vector2i& pos, const Vector2i& size) {
    VkRect2D scissor{};
    scissor.offset = VkOffset2D{pos.x, pos.y};
    scissor.extent = VkExtent2D{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::bindPipeline(const VulkanPipeline& pipeline) {
    if (pipeline.isCompute()) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
    } else {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
    }
}

void VulkanCommandBuffer::bindBuffers(const Span<VulkanVertexBufferBindRef>& buffers) {
    std::vector<VkBuffer> handles{};
    std::vector<VkDeviceSize> offsets{};

    handles.resize(buffers.size());
    offsets.resize(buffers.size());

    for (size_t i = 0; i < buffers.size(); i++) {
        handles[i] = buffers[i].buffer->getHandle();
        offsets[i] = buffers[i].offset;
    }

    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), handles.data(), offsets.data());
}

void VulkanCommandBuffer::draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
                               const uint32_t firstInstance) {
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(const uint32_t indexCount, const uint32_t instanceCount,
                                      const uint32_t firstIndex, const int32_t vertexOffset,
                                      const uint32_t firstInstance) {
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) {
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void VulkanCommandBuffer::copyImage(const VulkanTexture& src, const VkImageLayout srcLayout, const VulkanTexture& dst,
                                    const VkImageLayout dstLayout, const VkImageCopy& region) {
    vkCmdCopyImage(commandBuffer, src.getHandle(), srcLayout, dst.getHandle(), dstLayout, 1, &region);
}

void VulkanCommandBuffer::copyImageToBuffer(const VulkanTexture& src, const VkImageLayout srcLayout, VulkanBuffer& dst,
                                            const Span<VkBufferImageCopy>& regions) {
    vkCmdCopyImageToBuffer(commandBuffer, src.getHandle(), srcLayout, dst.getHandle(), regions.size(), regions.data());
}

void VulkanCommandBuffer::copyBuffer(const VulkanBuffer& src, const VulkanBuffer& dst, const VkBufferCopy& region) {
    vkCmdCopyBuffer(commandBuffer, src.getHandle(), dst.getHandle(), 1, &region);
}

void VulkanCommandBuffer::bindIndexBuffer(const VulkanBuffer& buffer, const VkDeviceSize offset,
                                          const VkIndexType indexType) {
    vkCmdBindIndexBuffer(commandBuffer, buffer.getHandle(), offset, indexType);
}

void VulkanCommandBuffer::copyBufferToImage(const VulkanBuffer& src, const VulkanTexture& dst,
                                            const VkBufferImageCopy& region) {
    vkCmdCopyBufferToImage(
        commandBuffer, src.getHandle(), dst.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanCommandBuffer::copyBufferToImage(const VulkanBuffer& src, const VulkanTexture& dst,
                                            const Span<VkBufferImageCopy>& regions) {
    vkCmdCopyBufferToImage(commandBuffer,
                           src.getHandle(),
                           dst.getHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           regions.size(),
                           regions.data());
}

void VulkanCommandBuffer::bindDescriptorSet(const VulkanDescriptorSet& descriptorSet, VkPipelineLayout pipelineLayout,
                                            const bool isCompute) {
    auto set = descriptorSet.getHandle();
    vkCmdBindDescriptorSets(commandBuffer,
                            isCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            0,
                            1,
                            &set,
                            0,
                            nullptr);
}

void VulkanCommandBuffer::pipelineBarrier(const VkPipelineStageFlags& source, const VkPipelineStageFlags& destination,
                                          VkImageMemoryBarrier& barrier) {
    vkCmdPipelineBarrier(commandBuffer, source, destination, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanCommandBuffer::bindDescriptors(const VulkanPipeline& pipeline, const VulkanDescriptorSetLayout& layout,
                                          const Span<VulkanBufferBinding>& uniforms,
                                          const Span<VulkanTextureBinding>& textures,
                                          const Span<VulkanTextureBinding>& inputs) {

    auto descriptorSet = descriptorPool->createDescriptorSet(layout);
    descriptorSet.bind(uniforms, textures, inputs);

    bindDescriptorSet(descriptorSet, pipeline.getLayout(), pipeline.isCompute());
}

void VulkanCommandBuffer::bindDescriptors(const VulkanPipeline& pipeline, const VulkanDescriptorSetLayout& layout,
                                          VulkanDescriptorPool& pool, const Span<VulkanBufferBinding>& uniforms,
                                          const Span<VulkanTextureBinding>& textures,
                                          const Span<VulkanTextureBinding>& inputs) {
    auto descriptorSet = pool.createDescriptorSet(layout);
    descriptorSet.bind(uniforms, textures, inputs);

    bindDescriptorSet(descriptorSet, pipeline.getLayout(), pipeline.isCompute());
}

void VulkanCommandBuffer::pushConstants(const VulkanPipeline& pipeline, const VkShaderStageFlags shaderStage,
                                        const size_t offset, const size_t size, const void* data) {
    vkCmdPushConstants(commandBuffer, pipeline.getLayout(), shaderStage, offset, size, data);
}

void VulkanCommandBuffer::blitImage(const VulkanTexture& src, VkImageLayout srcLayout, const VulkanTexture& dst,
                                    const VkImageLayout dstLayout, const Span<VkImageBlit>& regions,
                                    const VkFilter filter) {
    vkCmdBlitImage(
        commandBuffer, src.getHandle(), srcLayout, dst.getHandle(), dstLayout, regions.size(), regions.data(), filter);
}

void VulkanCommandBuffer::generateMipMaps(VulkanTexture& texture) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = texture.getHandle();
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = texture.getLayerCount();
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texture.getExtent().width;
    int32_t mipHeight = texture.getExtent().height;

    for (uint32_t i = 1; i < texture.getMipMaps(); i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = texture.getLayerCount();
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = texture.getLayerCount();

        std::array<VkImageBlit, 1> imageBlit{};
        imageBlit[0] = blit;
        blitImage(texture,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  texture,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  imageBlit,
                  VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = texture.getMipMaps() - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
}

void VulkanCommandBuffer::writeTimestamp(VulkanQueryPool& queryPool, const VkPipelineStageFlagBits stage,
                                         const uint32_t query) {
    vkCmdWriteTimestamp(commandBuffer, stage, queryPool.getHandle(), query);
}

void VulkanCommandBuffer::resetQueryPool(VulkanQueryPool& queryPool, const uint32_t firstQuery, const uint32_t count) {
    vkCmdResetQueryPool(commandBuffer, queryPool.getHandle(), firstQuery, count);
}
