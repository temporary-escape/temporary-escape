#include "vg_renderer.hpp"
#include "../utils/exceptions.hpp"

#define CMP "VgRenderer"

using namespace Engine;

VgRenderer::VgRenderer(const Config& config) :
    VgDevice{config}, config{config}, lastViewportSize{config.windowWidth, config.windowHeight} {

    const auto indices = getQueueFamilies();

    swapChain = VgSwapChain(*this, getSwapChainInfo());

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

    allocator = VgAllocator{config, *this};

    // Get required alignment flush size for selected physical device.
    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    vkGetPhysicalDeviceProperties(getPhysicalDevice(), &physicalDeviceProperties);
    alignedFlushSize = physicalDeviceProperties.limits.nonCoherentAtomSize;

    Log::i(CMP, "Using aligned flush size: {} bytes", alignedFlushSize);

    // Pinned buffer to be used for memory transfer
    VgBuffer::CreateInfo transferBufferCreateInfo{};
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

VgRenderer::~VgRenderer() {
    destroy();
}

void VgRenderer::destroy() {
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

    allocator.destroy();
}

void VgRenderer::getGpuMemoryStats() {
    VmaBudget info;
    vmaGetBudget(allocator.getHandle(), &info);
    Log::d(CMP, "Allocator budget: {} usage: {}", info.budget, info.usage);
}

void VgRenderer::onNextFrame() {
    getCurrentSyncObject().wait();

    getCurrentDescriptorPool().reset();

    // Destroy buffers we no longer need
    destroyDisposables();

    // Grab the next swap image index
    const auto result = vkAcquireNextImageKHR(getDevice(), swapChain.getHandle(), UINT64_MAX,
                                              getCurrentSyncObject().getImageAvailableSemaphore(), VK_NULL_HANDLE,
                                              &swapChainFramebufferIndex);

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

void VgRenderer::onFrameDraw(const Vector2i& viewport, float deltaTime) {
    // auto& commandBuffer = getCommandBuffer();

    // VkCommandBufferBeginInfo beginInfo{};
    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // commandBuffer.start(beginInfo);

    render(viewport, deltaTime);

    // commandBuffer.end();
    // submitCommandBuffer(commandBuffer);

    submitPresentQueue();

    // Do we need to recreate the swap chain?
    if (lastViewportSize != viewport) {
        recreateSwapChain();
    }

    lastViewportSize = viewport;
}

void VgRenderer::onExit() {
    waitDeviceIdle();
    exitTriggered = true;
}

VgShaderModule VgRenderer::createShaderModule(const Path& path, VkShaderStageFlagBits stage) {
    return VgShaderModule{config, *this, path, stage};
}

VgShaderModule VgRenderer::createShaderModule(const std::string& glsl, VkShaderStageFlagBits stage) {
    return VgShaderModule{config, *this, glsl, stage};
}

VgPipeline VgRenderer::createPipeline(const VgRenderPass& renderPass, const VgPipeline::CreateInfo& createInfo) {
    return VgPipeline{config, *this, renderPass, createInfo};
}

VgRenderPass VgRenderer::createRenderPass(const VgRenderPass::CreateInfo& createInfo) {
    return VgRenderPass{config, *this, createInfo};
}

VgFramebuffer VgRenderer::createFramebuffer(const VgFramebuffer::CreateInfo& createInfo) {
    return VgFramebuffer{config, *this, createInfo};
}

VgSyncObject VgRenderer::createSyncObject() {
    return VgSyncObject{config, *this};
}

VgCommandPool VgRenderer::createCommandPool(const VgCommandPool::CreateInfo& createInfo) {
    return VgCommandPool{config, *this, createInfo};
}

VgCommandBuffer VgRenderer::createCommandBuffer() {
    return VgCommandBuffer{config, *this, commandPool};
}

VgBuffer VgRenderer::createBuffer(const VgBuffer::CreateInfo& createInfo) {
    return VgBuffer{config, *this, createInfo};
}

VgDescriptorSetLayout VgRenderer::createDescriptorSetLayout(const VgDescriptorSetLayout::CreateInfo& createInfo) {
    return VgDescriptorSetLayout{config, *this, createInfo};
}

VgDescriptorPool VgRenderer::createDescriptorPool() {
    return VgDescriptorPool{config, *this};
}

VgDescriptorSetLayout VgRenderer::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    return VgDescriptorSetLayout{config, *this, layoutInfo};
}

VgDescriptorSet VgRenderer::createDescriptorSet(VgDescriptorPool& pool, VgDescriptorSetLayout& layout) {
    return VgDescriptorSet{config, *this, pool, layout};
}

VgDoubleBuffer VgRenderer::createDoubleBuffer(const VgBuffer::CreateInfo& createInfo) {
    return VgDoubleBuffer{config, *this, createInfo};
}

VgTexture VgRenderer::createTexture(const VgTexture::CreateInfo& createInfo) {
    return VgTexture{config, *this, createInfo};
}

void VgRenderer::waitDeviceIdle() {
    vkDeviceWaitIdle(getDevice());
}

void VgRenderer::submitCommandBuffer(const VgCommandBuffer& commandBuffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {getCurrentSyncObject().getImageAvailableSemaphore()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    const auto commandBufferHandle = commandBuffer.getHandle();

    VkSemaphore signalSemaphores[] = {getCurrentSyncObject().getRenderFinishedSemaphore()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBufferHandle;

    if (vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, getCurrentSyncObject().getHandle()) != VK_SUCCESS) {
        EXCEPTION("Failed to submit draw command buffer!");
    }
}

void VgRenderer::submitPresentQueue() {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = {getCurrentSyncObject().getRenderFinishedSemaphore()};

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

void VgRenderer::recreateSwapChain() {
    waitUntilValidFramebufferSize();

    waitDeviceIdle();

    swapChain.destroy();
    swapChain = VgSwapChain{*this, getSwapChainInfo()};

    createSwapChainFramebuffers();
}

void VgRenderer::uploadBufferData(const void* data, size_t size, VgBuffer& dst) {
    // Adapted from: https://github.com/GPUOpen-LibrariesAndSDKs/V-EZ/blob/master/Source/Core/Device.cpp

    if (!transferBuffer.getMappedPtr()) {
        EXCEPTION("Transfer buffer has no mapped memory!");
    }

    auto transferCommandBuffer = getCommandPool().createCommandBuffer();

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
        vmaGetAllocationInfo(allocator.getHandle(), transferBuffer.getAllocation(), &allocInfo);

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
        transferCommandBuffer.copyBuffer(transferBuffer, dst, copyRegion);

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

    transferCommandBuffer.free();
}

void VgRenderer::transitionImageLayout(VgTexture& texture, const VkFormat format, const VkImageLayout oldLayout,
                                       const VkImageLayout newLayout) {

    auto transferCommandBuffer = getCommandPool().createCommandBuffer();

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
    barrier.subresourceRange.levelCount = 1;
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
        EXCEPTION("Failed to upload buffer data, submit error");
    }

    if (vkQueueWaitIdle(getGraphicsQueue()) != VK_SUCCESS) {
        EXCEPTION("Failed to upload buffer data, wait queue error");
    }

    transferCommandBuffer.free();
}

void VgRenderer::copyBufferToImage(const VgBuffer& buffer, VgTexture& texture, const int level, const int layer,
                                   const VkOffset3D& offset, const VkExtent3D& extent) {

    auto transferCommandBuffer = getCommandPool().createCommandBuffer();

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

    transferCommandBuffer.free();
}

void VgRenderer::createRenderPass() {
    VgRenderPass::CreateInfo renderPassInfo{};

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

    renderPass = VgRenderer::createRenderPass(renderPassInfo);
}

void VgRenderer::createSwapChainFramebuffers() {
    const auto& swapChainImageViews = getSwapChain().getImageViews();

    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VgFramebuffer::CreateInfo framebufferInfo{};
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

void VgRenderer::dispose(std::shared_ptr<VgDisposable> disposable) {
    if (exitTriggered) {
        disposable->destroy();
        return;
    }
    disposables.at(getCurrentFrameNum()).push_back(std::move(disposable));
}

void VgRenderer::destroyDisposables() {
    for (auto& disposable : disposables.at(getCurrentFrameNum())) {
        disposable->destroy();
    }
    disposables.at(getCurrentFrameNum()).clear();
}

void VgRenderer::destroyDisposablesAll() {
    for (auto& currentFrameDisposables : disposables) {
        for (auto& disposable : currentFrameDisposables) {
            disposable->destroy();
        }
        currentFrameDisposables.clear();
    }
}
