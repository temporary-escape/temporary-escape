#include "vg_command_buffer.hpp"
#include "../utils/exceptions.hpp"
#include "vg_command_pool.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgCommandBuffer::VgCommandBuffer(const Config& config, VgDevice& device, VgCommandPool& commandPool) : device{&device} {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool.getHandle();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(device.getHandle(), &allocInfo, commandBuffers) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate command buffers!");
    }

    commandBuffer = commandBuffers[0];
}

VgCommandBuffer::~VgCommandBuffer() {
    destroy();
}

VgCommandBuffer::VgCommandBuffer(VgCommandBuffer&& other) noexcept {
    swap(other);
}

VgCommandBuffer& VgCommandBuffer::operator=(VgCommandBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgCommandBuffer::swap(VgCommandBuffer& other) noexcept {
    std::swap(device, other.device);
    std::swap(commandBuffers, other.commandBuffers);
    std::swap(commandBuffer, other.commandBuffer);
}

void VgCommandBuffer::destroy() {
    commandBuffer = VK_NULL_HANDLE;
}

void VgCommandBuffer::startCommandBuffer(const VkCommandBufferBeginInfo& beginInfo) {
    commandBuffer = commandBuffers[device->getCurrentFrameNum()];

    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    /*VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (oneTimeSubmit) {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }*/

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        EXCEPTION("Failed to begin recording command buffer!");
    }
}

void VgCommandBuffer::endCommandBuffer() {
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to record command buffer!");
    }
}

void VgCommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo) {
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VgCommandBuffer::endRenderPass() {
    vkCmdEndRenderPass(commandBuffer);
}

void VgCommandBuffer::setViewport(const Vector2i& pos, const Vector2i& size, const float minDepth,
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

void VgCommandBuffer::setScissor(const Vector2i& pos, const Vector2i& size) {
    VkRect2D scissor{};
    scissor.offset = VkOffset2D{pos.x, pos.y};
    scissor.extent = VkExtent2D{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VgCommandBuffer::bindPipeline(const VgPipeline& pipeline) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
}

void VgCommandBuffer::bindBuffers(const std::vector<VgVertexBufferBindRef>& buffers) {
    std::vector<VkBuffer> handles{buffers.size()};
    std::vector<VkDeviceSize> offsets{buffers.size()};
    for (size_t i = 0; i < buffers.size(); i++) {
        handles.at(i) = buffers.at(i).buffer.get().getHandle();
        offsets.at(i) = buffers.at(i).offset;
    }

    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), handles.data(), offsets.data());
}

void VgCommandBuffer::drawVertices(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
                                   const uint32_t firstInstance) {
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VgCommandBuffer::copyBuffer(const VgBuffer& src, const VgBuffer& dst, const VkBufferCopy& region) {
    vkCmdCopyBuffer(commandBuffer, src.getHandle(), dst.getHandle(), 1, &region);
}
