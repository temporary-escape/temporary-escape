#include "vulkan_command_buffer.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_command_pool.hpp"
#include "vulkan_descriptor_pool.hpp"
#include "vulkan_descriptor_set.hpp"
#include "vulkan_descriptor_set_layout.hpp"
#include "vulkan_device.hpp"
#include "vulkan_framebuffer.hpp"
#include "vulkan_pipeline.hpp"

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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
}

void VulkanCommandBuffer::bindBuffers(const std::vector<VulkanVertexBufferBindRef>& buffers) {
    std::vector<VkBuffer> handles{buffers.size()};
    std::vector<VkDeviceSize> offsets{buffers.size()};
    for (size_t i = 0; i < buffers.size(); i++) {
        handles.at(i) = buffers.at(i).buffer.get().getHandle();
        offsets.at(i) = buffers.at(i).offset;
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

void VulkanCommandBuffer::copyBuffer(const VulkanBuffer& src, const VulkanBuffer& dst, const VkBufferCopy& region) {
    vkCmdCopyBuffer(commandBuffer, src.getHandle(), dst.getHandle(), 1, &region);
}

void VulkanCommandBuffer::bindIndexBuffer(const VulkanBuffer& buffer, const VkDeviceSize offset,
                                          const VkIndexType indexType) {
    vkCmdBindIndexBuffer(commandBuffer, buffer.getHandle(), offset, indexType);
}

void VulkanCommandBuffer::copyBufferToImage(const VulkanBuffer& src, const VulkanTexture& dst,
                                            const VkBufferImageCopy& region) {
    vkCmdCopyBufferToImage(commandBuffer, src.getHandle(), dst.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);
}

void VulkanCommandBuffer::bindDescriptorSet(const VulkanDescriptorSet& descriptorSet, VkPipelineLayout pipelineLayout) {
    auto set = descriptorSet.getHandle();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);
}

void VulkanCommandBuffer::pipelineBarrier(const VkPipelineStageFlags& source, const VkPipelineStageFlags& destination,
                                          VkImageMemoryBarrier& barrier) {
    vkCmdPipelineBarrier(commandBuffer, source, destination, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanCommandBuffer::bindDescriptors(VulkanPipeline& pipeline, VulkanDescriptorSetLayout& layout,
                                          const std::vector<VulkanBufferBinding>& uniforms,
                                          const std::vector<VulkanTextureBinding>& textures) {

    auto descriptorSet = descriptorPool->createDescriptorSet(layout);
    descriptorSet.bind(uniforms, textures);

    bindDescriptorSet(descriptorSet, pipeline.getLayout());
}

void VulkanCommandBuffer::pushConstants(VulkanPipeline& pipeline, const VkShaderStageFlags shaderStage,
                                        const size_t offset, const size_t size, const void* data) {
    vkCmdPushConstants(commandBuffer, pipeline.getLayout(), shaderStage, offset, size, data);
}
