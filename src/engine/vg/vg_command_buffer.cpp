#include "vg_command_buffer.hpp"
#include "../utils/exceptions.hpp"
#include "vg_command_pool.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgCommandBuffer::VgCommandBuffer(const Config& config, VgDevice& device, VgCommandPool& commandPool) :
    state{std::make_shared<CommandBufferState>()} {

    state->device = &device;
    state->commandPool = commandPool.getHandle();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool.getHandle();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device.getHandle(), &allocInfo, &state->commandBuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate command buffer!");
    }
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
    std::swap(state, other.state);
}

void VgCommandBuffer::destroy() {
    if (state && state->device) {
        state->device->dispose(state);
    }

    state.reset();
}

void VgCommandBuffer::free() {
    if (state) {
        state->destroy();
    }
    state.reset();
}

void VgCommandBuffer::startCommandBuffer(const VkCommandBufferBeginInfo& beginInfo) {
    vkResetCommandBuffer(state->commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    if (vkBeginCommandBuffer(state->commandBuffer, &beginInfo) != VK_SUCCESS) {
        EXCEPTION("Failed to begin recording command buffer!");
    }
}

void VgCommandBuffer::endCommandBuffer() {
    if (vkEndCommandBuffer(state->commandBuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to record command buffer!");
    }
}

void VgCommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo) {
    vkCmdBeginRenderPass(state->commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VgCommandBuffer::endRenderPass() {
    vkCmdEndRenderPass(state->commandBuffer);
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
    vkCmdSetViewport(state->commandBuffer, 0, 1, &viewport);
}

void VgCommandBuffer::setScissor(const Vector2i& pos, const Vector2i& size) {
    VkRect2D scissor{};
    scissor.offset = VkOffset2D{pos.x, pos.y};
    scissor.extent = VkExtent2D{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};
    vkCmdSetScissor(state->commandBuffer, 0, 1, &scissor);
}

void VgCommandBuffer::bindPipeline(const VgPipeline& pipeline) {
    vkCmdBindPipeline(state->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
}

void VgCommandBuffer::bindBuffers(const std::vector<VgVertexBufferBindRef>& buffers) {
    std::vector<VkBuffer> handles{buffers.size()};
    std::vector<VkDeviceSize> offsets{buffers.size()};
    for (size_t i = 0; i < buffers.size(); i++) {
        handles.at(i) = buffers.at(i).buffer.get().getHandle();
        offsets.at(i) = buffers.at(i).offset;
    }

    vkCmdBindVertexBuffers(state->commandBuffer, 0, buffers.size(), handles.data(), offsets.data());
}

void VgCommandBuffer::drawVertices(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
                                   const uint32_t firstInstance) {
    vkCmdDraw(state->commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VgCommandBuffer::copyBuffer(const VgBuffer& src, const VgBuffer& dst, const VkBufferCopy& region) {
    vkCmdCopyBuffer(state->commandBuffer, src.getHandle(), dst.getHandle(), 1, &region);
}

void VgCommandBuffer::copyBufferToImage(const VgBuffer& src, const VgTexture& dst, const VkBufferImageCopy& region) {
    vkCmdCopyBufferToImage(state->commandBuffer, src.getHandle(), dst.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region);
}

void VgCommandBuffer::bindDescriptorSet(const VgDescriptorSet& descriptorSet, VkPipelineLayout pipelineLayout) {
    auto set = descriptorSet.getHandle();
    vkCmdBindDescriptorSets(state->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0,
                            nullptr);
}

void VgCommandBuffer::pipelineBarrier(const VkPipelineStageFlags& source, const VkPipelineStageFlags& destination,
                                      VkImageMemoryBarrier& barrier) {
    vkCmdPipelineBarrier(state->commandBuffer, source, destination, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VgCommandBuffer::CommandBufferState::destroy() {
    if (commandBuffer) {
        vkFreeCommandBuffers(device->getHandle(), commandPool, 1, &commandBuffer);
        commandBuffer = VK_NULL_HANDLE;
    }
}
