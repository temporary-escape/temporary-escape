#include "vulkan_renderer.hpp"
#include "../utils/exceptions.hpp"

#define CMP "VulkanRenderer"

using namespace Engine;

VulkanRenderer::VulkanRenderer(const Config& config) :
    VulkanDevice{config}, config{config}, lastViewportSize{config.windowWidth, config.windowHeight} {

    const auto indices = getQueueFamilies();

    swapChain = VulkanSwapChain(*this, getSwapChainInfo());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    commandPool = createCommandPool(poolInfo);

    for (auto& syncObject : syncObjects) {
        syncObject = createSyncObject();
    }

    for (auto& descriptorPool : descriptorPools) {
        descriptorPool = createDescriptorPool();
    }

    // Get required alignment flush size for selected physical device.
    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    vkGetPhysicalDeviceProperties(getPhysicalDevice(), &physicalDeviceProperties);
    alignedFlushSize = physicalDeviceProperties.limits.nonCoherentAtomSize;

    Log::i(CMP, "Using aligned flush size: {} bytes", alignedFlushSize);

    // Pinned buffer to be used for memory transfer
    VulkanBuffer::CreateInfo transferBufferCreateInfo{};
    transferBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    transferBufferCreateInfo.size = 8 * 1024 * 1024;
    transferBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    transferBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    transferBufferCreateInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    transferBufferCreateInfo.memoryFlags =
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    transferBuffer = createBuffer(transferBufferCreateInfo);

    Log::i(CMP, "Using buffer flush size: {} bytes", transferBuffer.getSize());

    createRenderPass();
    createSwapChainFramebuffers();
}

VulkanRenderer::~VulkanRenderer() {
    destroy();
}

void VulkanRenderer::destroy() {
    renderPass.destroy();

    for (auto& framebuffer : swapChainFramebuffers) {
        framebuffer.destroy();
    }
    swapChainFramebuffers.clear();

    destroyDisposablesAll();

    commandPool.destroy();

    swapChain.destroy();

    for (auto& syncObject : syncObjects) {
        syncObject.destroy();
    }

    for (auto& descriptorPool : descriptorPools) {
        descriptorPool.destroy();
    }

    transferBuffer.destroy();
}

/*void VulkanRenderer::getGpuMemoryStats() {
    VmaBudget info;
    vmaGetBudget(allocator.getHandle(), &info);
    Log::d(CMP, "Allocator budget: {} usage: {}", info.budget, info.usage);
}*/

void VulkanRenderer::onNextFrame() {
    getCurrentSyncObject().wait();

    getCurrentDescriptorPool().reset();

    // Destroy buffers we no longer need
    destroyDisposables();

    // Grab the next swap image index
    const auto result = vkAcquireNextImageKHR(getDevice(), swapChain.getHandle(), UINT64_MAX,
                                              getCurrentSyncObject().getImageAvailableSemaphore().getHandle(),
                                              VK_NULL_HANDLE, &swapChainFramebufferIndex);

    // Do we need to recreate the swap chain?
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }

    // Something else happened?
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        EXCEPTION("Failed to acquire swap chain image!");
    }

    getCurrentSyncObject().reset();
}

void VulkanRenderer::onFrameDraw(const Vector2i& viewport, float deltaTime) {
    // auto& commandBuffer = getCommandBuffer();

    // VkCommandBufferBeginInfo beginInfo{};
    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // commandBuffer.start(beginInfo);

    try {
        render(viewport, deltaTime);
    } catch (...) {
        waitDeviceIdle();
        EXCEPTION_NESTED("Exception thrown during render");
    }
    // commandBuffer.end();
    // submitPresentCommandBuffer(commandBuffer);

    submitPresentQueue();

    // Do we need to recreate the swap chain?
    if (lastViewportSize != viewport) {
        recreateSwapChain();
    }

    lastViewportSize = viewport;
}

void VulkanRenderer::onExit() {
    waitDeviceIdle();
    exitTriggered = true;
}

VulkanShaderModule VulkanRenderer::createShaderModule(const Path& path, VkShaderStageFlagBits stage) {
    return VulkanShaderModule{config, *this, path, stage};
}

VulkanShaderModule VulkanRenderer::createShaderModule(const std::string& glsl, VkShaderStageFlagBits stage) {
    return VulkanShaderModule{config, *this, glsl, stage};
}

VulkanPipeline VulkanRenderer::createPipeline(const VulkanRenderPass& renderPass,
                                              const VulkanPipeline::CreateInfo& createInfo) {
    return VulkanPipeline{*this, renderPass, createInfo};
}

VulkanRenderPass VulkanRenderer::createRenderPass(const VulkanRenderPass::CreateInfo& createInfo) {
    return VulkanRenderPass{*this, createInfo};
}

VulkanFramebuffer VulkanRenderer::createFramebuffer(const VulkanFramebuffer::CreateInfo& createInfo) {
    return VulkanFramebuffer{*this, createInfo};
}

VulkanSyncObject VulkanRenderer::createSyncObject() {
    return VulkanSyncObject{*this};
}

VulkanSemaphore VulkanRenderer::createSemaphore(const VulkanSemaphore::CreateInfo& createInfo) {
    return VulkanSemaphore{*this, createInfo};
}

VulkanCommandPool VulkanRenderer::createCommandPool(const VulkanCommandPool::CreateInfo& createInfo) {
    return VulkanCommandPool{*this, createInfo};
}

VulkanCommandBuffer VulkanRenderer::createCommandBuffer() {
    return VulkanCommandBuffer{*this, commandPool, getCurrentDescriptorPool()};
}

VulkanBuffer VulkanRenderer::createBuffer(const VulkanBuffer::CreateInfo& createInfo) {
    return VulkanBuffer{*this, createInfo};
}

VulkanDescriptorSetLayout
VulkanRenderer::createDescriptorSetLayout(const VulkanDescriptorSetLayout::CreateInfo& createInfo) {
    return VulkanDescriptorSetLayout{*this, createInfo};
}

VulkanDescriptorPool VulkanRenderer::createDescriptorPool() {
    return VulkanDescriptorPool{*this};
}

VulkanDescriptorSetLayout
VulkanRenderer::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    return VulkanDescriptorSetLayout{*this, layoutInfo};
}

VulkanDoubleBuffer VulkanRenderer::createDoubleBuffer(const VulkanBuffer::CreateInfo& createInfo) {
    return VulkanDoubleBuffer{*this, createInfo};
}

VulkanTexture VulkanRenderer::createTexture(const VulkanTexture::CreateInfo& createInfo) {
    return VulkanTexture{*this, createInfo};
}

void VulkanRenderer::waitDeviceIdle() {
    vkDeviceWaitIdle(getDevice());
}

void VulkanRenderer::submitCommandBuffer(const VulkanCommandBuffer& commandBuffer,
                                         const VkPipelineStageFlags waitStages, VulkanSemaphore& wait,
                                         VulkanSemaphore& signal) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.getHandle();

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &wait.getHandle();
    submitInfo.pWaitDstStageMask = &waitStages;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signal.getHandle();

    if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        EXCEPTION("Failed to submit command buffer");
    }
}

void VulkanRenderer::submitPresentCommandBuffer(const VulkanCommandBuffer& commandBuffer, VulkanSemaphore* wait) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {wait ? wait->getHandle()
                                         : getCurrentSyncObject().getImageAvailableSemaphore().getHandle()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    const auto commandBufferHandle = commandBuffer.getHandle();

    VkSemaphore signalSemaphores[] = {getCurrentSyncObject().getRenderFinishedSemaphore().getHandle()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBufferHandle;

    if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, getCurrentSyncObject().getHandle()) != VK_SUCCESS) {
        EXCEPTION("Failed to submit draw command buffer!");
    }
}

void VulkanRenderer::submitPresentQueue() {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = {getCurrentSyncObject().getRenderFinishedSemaphore().getHandle()};

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &swapChainFramebufferIndex;

    const auto result = vkQueuePresentKHR(getPresentQueue(), &presentInfo);

    // Do we need to recreate the swap chain?
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
        return;
    }

    // Something else happened?
    else if (result != VK_SUCCESS) {
        EXCEPTION("Failed to present swap chain image!");
    }

    currentFrameNum = (currentFrameNum + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::recreateSwapChain() {
    waitUntilValidFramebufferSize();

    waitDeviceIdle();

    swapChain.destroy();
    swapChain = VulkanSwapChain{*this, getSwapChainInfo()};

    createSwapChainFramebuffers();
}

void VulkanRenderer::copyDataToBuffer(VulkanBuffer& buffer, const void* data, size_t size) {
    // Adapted from: https://github.com/GPUOpen-LibrariesAndSDKs/V-EZ/blob/master/Source/Core/Device.cpp

    if (!transferBuffer.getMappedPtr()) {
        EXCEPTION("Transfer buffer has no mapped memory!");
    }

    auto transferCommandBuffer = createCommandBuffer();

    auto src = reinterpret_cast<const char*>(data);
    VkDeviceSize dstOffset = 0;
    auto bytesRemaining = size;

    while (bytesRemaining) {
        // Determine total byte size to copy this iteration.
        auto bytesToCopy = std::min(transferBuffer.getSize(), bytesRemaining);

        std::memcpy(transferBuffer.getMappedPtr(), src, bytesToCopy);

        // Flush must be aligned according to physical device's limits.
        auto alignedBytesToCopy = static_cast<VkDeviceSize>(std::ceil(static_cast<float>(bytesToCopy) /
                                                                      static_cast<float>(alignedFlushSize))) *
                                  alignedFlushSize;

        // Flush the memory write.
        VmaAllocationInfo allocInfo = {};
        vmaGetAllocationInfo(getAllocator().getHandle(), transferBuffer.getAllocation(), &allocInfo);

        VkMappedMemoryRange memoryRange = {};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = allocInfo.deviceMemory;
        memoryRange.offset = 0;
        memoryRange.size = alignedBytesToCopy;
        if (vkFlushMappedMemoryRanges(getDevice(), 1, &memoryRange) != VK_SUCCESS) {
            EXCEPTION("Failed to upload buffer data, flush mapped memory ranges error");
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        transferCommandBuffer.start(beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = dstOffset;
        transferCommandBuffer.copyBuffer(transferBuffer, buffer, copyRegion);

        transferCommandBuffer.end();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &transferCommandBuffer.getHandle();

        if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            EXCEPTION("Failed to upload buffer data, submit error");
        }

        if (vkQueueWaitIdle(getGraphicsQueue()) != VK_SUCCESS) {
            EXCEPTION("Failed to upload buffer data, wait queue error");
        }

        // Update running counters
        bytesRemaining -= bytesToCopy;
        src += bytesToCopy;
        dstOffset += bytesToCopy;
    }
}

void VulkanRenderer::transitionImageLayout(VulkanTexture& texture, const VkImageLayout oldLayout,
                                           const VkImageLayout newLayout) {

    auto transferCommandBuffer = createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    transferCommandBuffer.start(beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        EXCEPTION("Unsupported layout transition during image layout transition!");
    }

    transferCommandBuffer.pipelineBarrier(sourceStage, destinationStage, barrier);

    transferCommandBuffer.end();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferCommandBuffer.getHandle();

    if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        EXCEPTION("Failed to transition image layout, submit error");
    }

    if (vkQueueWaitIdle(getGraphicsQueue()) != VK_SUCCESS) {
        EXCEPTION("Failed to transition image layout, wait queue error");
    }
}

void VulkanRenderer::waitQueueIdle() {
    if (vkQueueWaitIdle(getGraphicsQueue()) != VK_SUCCESS) {
        EXCEPTION("Failed to upload buffer data, wait queue error");
    }
}

void VulkanRenderer::submitCommandBuffer(const VulkanCommandBuffer& commandBuffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.getHandle();

    if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        EXCEPTION("Failed to submit command buffer");
    }
}

void VulkanRenderer::copyDataToImage(VulkanTexture& texture, int level, const Vector2i& offset, int layer,
                                     const Vector2i& size, const void* data) {

    auto region = VkExtent3D{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = getFormatDataSize(texture.getFormat(), region);
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;

    auto stagingBuffer = createBuffer(bufferInfo);
    // stagingBuffer.subData(data, 0, bufferInfo.size);
    auto* stagingBufferDst = stagingBuffer.mapMemory();
    std::memcpy(stagingBufferDst, data, bufferInfo.size);
    stagingBuffer.unmapMemory();

    copyBufferToImage(stagingBuffer, texture, level, layer, {offset.x, offset.y, 0}, region);
}

void VulkanRenderer::copyBufferToImage(const VulkanBuffer& buffer, VulkanTexture& texture, const int level,
                                       const int layer, const VkOffset3D& offset, const VkExtent3D& extent) {

    auto transferCommandBuffer = createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    transferCommandBuffer.start(beginInfo);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = level;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = offset;
    region.imageExtent = extent;

    transferCommandBuffer.copyBufferToImage(buffer, texture, region);

    transferCommandBuffer.end();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferCommandBuffer.getHandle();

    if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        EXCEPTION("Failed to upload buffer data, submit error");
    }

    if (vkQueueWaitIdle(getGraphicsQueue()) != VK_SUCCESS) {
        EXCEPTION("Failed to upload buffer data, wait queue error");
    }
}

void VulkanRenderer::generateMipMaps(VulkanTexture& texture) {
    const auto formatProperties = getPhysicalDeviceFormatProperties(texture.getFormat());

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        EXCEPTION("Texture image format does not support linear blitting!");
    }

    auto commandBuffer = createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBuffer.start(beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = texture.getHandle();
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texture.getExtent().width;
    int32_t mipHeight = texture.getExtent().height;

    for (uint32_t i = 1; i < texture.getMipMaps(); i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        commandBuffer.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        commandBuffer.blitImage(texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {blit}, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        commandBuffer.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

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

    commandBuffer.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

    commandBuffer.end();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.getHandle();

    if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        EXCEPTION("Failed to upload buffer data, submit error");
    }

    if (vkQueueWaitIdle(getGraphicsQueue()) != VK_SUCCESS) {
        EXCEPTION("Failed to upload buffer data, wait queue error");
    }
}

void VulkanRenderer::createRenderPass() {
    VulkanRenderPass::CreateInfo renderPassInfo{};

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = getSwapChain().getFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassInfo.attachments = {colorAttachment};

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    renderPassInfo.subPasses = {subpass};

    renderPass = VulkanRenderer::createRenderPass(renderPassInfo);
}

void VulkanRenderer::createSwapChainFramebuffers() {
    const auto& swapChainImageViews = getSwapChain().getImageViews();

    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VulkanFramebuffer::CreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.getHandle();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = getSwapChain().getExtent().width;
        framebufferInfo.height = getSwapChain().getExtent().height;
        framebufferInfo.layers = 1;

        swapChainFramebuffers[i] = createFramebuffer(framebufferInfo);
    }
}

void VulkanRenderer::destroyDisposables() {
    for (auto& disposable : disposables.at(getCurrentFrameNum())) {
        disposable->destroy();
    }
    disposables.at(getCurrentFrameNum()).clear();
}

void VulkanRenderer::destroyDisposablesAll() {
    for (auto& currentFrameDisposables : disposables) {
        for (auto& disposable : currentFrameDisposables) {
            disposable->destroy();
        }
        currentFrameDisposables.clear();
    }
}
