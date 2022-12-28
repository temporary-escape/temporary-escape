// clang-format off
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1001000
#include <vk_mem_alloc.h>
// clang-format on
#include "vg_device.hpp"
#include "../utils/exceptions.hpp"

#define CMP "VgDevice"

using namespace Engine;

VgDevice::VgDevice(const Config& config) : VgInstance{config}, config{config} {
    const auto indices = getQueueFamilies();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(vgDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = vgDeviceExtensions.data();

    if (config.vulkan.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(vgValidationLayers.size());
        createInfo.ppEnabledLayerNames = vgValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(getPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS) {
        cleanup();
        EXCEPTION("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    swapChain = VgSwapChain(device, getSwapChainInfo());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    commandPool = createCommandPool(poolInfo);

    for (auto& syncObject : syncObjects) {
        syncObject = createSyncObject();
    }

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = getPhysicalDevice();
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = getInstance();

    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
        cleanup();
        EXCEPTION("Failed to create VMA allocator!");
    }

    // Get required alignment flush size for selected physical device.
    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    vkGetPhysicalDeviceProperties(getPhysicalDevice(), &physicalDeviceProperties);
    alignedFlushSize = physicalDeviceProperties.limits.nonCoherentAtomSize;

    Log::i(CMP, "Using aligned flush size: {} bytes", alignedFlushSize);

    // Allocate command buffer for memory transfers only
    transferCommandBuffer = commandPool.createCommandBuffer();

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
}

VgDevice::~VgDevice() {
    cleanup();
}

void VgDevice::cleanup() {
    destroyDisposables();

    transferCommandBuffer.destroy();

    commandPool.destroy();

    swapChain.destroy();
    for (auto& syncObject : syncObjects) {
        syncObject.destroy();
    }

    transferBuffer.destroy();

    if (allocator) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }

    if (device) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
}

void VgDevice::getGpuMemoryStats() {
    VmaBudget info;
    vmaGetBudget(allocator, &info);
    Log::d(CMP, "Allocator budget: {} usage: {}", info.budget, info.usage);
}

void VgDevice::onNextFrame() {
    getCurrentSyncObject().wait();

    // Destroy buffers we no longer need
    destroyDisposables();

    // Grab the next swap image index
    const auto result = vkAcquireNextImageKHR(device, swapChain.getHandle(), UINT64_MAX,
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

void VgDevice::onExit() {
    waitDeviceIdle();
    exitTriggered = true;
}

VgShaderModule VgDevice::createShaderModule(const Path& path, VkShaderStageFlagBits stage) {
    return VgShaderModule{config, device, path, stage};
}

VgShaderModule VgDevice::createShaderModule(const std::string& glsl, VkShaderStageFlagBits stage) {
    return VgShaderModule{config, device, glsl, stage};
}

VgPipeline VgDevice::createPipeline(const VgRenderPass& renderPass, const VgPipeline::CreateInfo& createInfo) {
    return VgPipeline{config, device, renderPass, createInfo};
}

VgRenderPass VgDevice::createRenderPass(const VgRenderPass::CreateInfo& createInfo) {
    return VgRenderPass{config, device, createInfo};
}

VgFramebuffer VgDevice::createFramebuffer(const VgFramebuffer::CreateInfo& createInfo) {
    return VgFramebuffer{config, device, createInfo};
}

VgSyncObject VgDevice::createSyncObject() {
    return VgSyncObject{config, device};
}

VgCommandPool VgDevice::createCommandPool(const VgCommandPool::CreateInfo& createInfo) {
    return VgCommandPool{config, *this, createInfo};
}

VgBuffer VgDevice::createBuffer(const VgBuffer::CreateInfo& createInfo) {
    return VgBuffer{config, *this, createInfo};
}

void VgDevice::waitDeviceIdle() {
    vkDeviceWaitIdle(device);
}

void VgDevice::submitCommandBuffer(const VgCommandBuffer& commandBuffer) {
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

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, getCurrentSyncObject().getHandle()) != VK_SUCCESS) {
        EXCEPTION("Failed to submit draw command buffer!");
    }
}

void VgDevice::submitPresentQueue() {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = {getCurrentSyncObject().getRenderFinishedSemaphore()};

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &swapChainFramebufferIndex;

    const auto result = vkQueuePresentKHR(presentQueue, &presentInfo);

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

void VgDevice::recreateSwapChain() {
    waitUntilValidFramebufferSize();

    waitDeviceIdle();

    swapChain.destroy();
    swapChain = VgSwapChain(device, getSwapChainInfo());

    onSwapChainChanged();
}

void VgDevice::uploadBufferData(const void* data, size_t size, VgBuffer& dst) {
    // Adapted from: https://github.com/GPUOpen-LibrariesAndSDKs/V-EZ/blob/master/Source/Core/Device.cpp

    if (!transferBuffer.getMappedPtr()) {
        EXCEPTION("Transfer buffer has no mapped memory!");
    }

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
        vmaGetAllocationInfo(allocator, transferBuffer.getAllocation(), &allocInfo);

        VkMappedMemoryRange memoryRange = {};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = allocInfo.deviceMemory;
        memoryRange.offset = 0;
        memoryRange.size = alignedBytesToCopy;
        if (vkFlushMappedMemoryRanges(device, 1, &memoryRange) != VK_SUCCESS) {
            EXCEPTION("Failed to upload buffer data, flush mapped memory ranges error");
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        transferCommandBuffer.startCommandBuffer(beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = dstOffset;
        transferCommandBuffer.copyBuffer(transferBuffer, dst, copyRegion);

        transferCommandBuffer.endCommandBuffer();

        auto commandBuffer = transferCommandBuffer.getHandle();
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            EXCEPTION("Failed to upload buffer data, submit error");
        }

        if (vkQueueWaitIdle(graphicsQueue) != VK_SUCCESS) {
            EXCEPTION("Failed to upload buffer data, wait queue error");
        }

        // Update running counters
        bytesRemaining -= bytesToCopy;
        src += bytesToCopy;
        dstOffset += bytesToCopy;
    }
}

void VgDevice::dispose(std::shared_ptr<VgDisposable> disposable) {
    if (exitTriggered) {
        disposable->destroy();
        return;
    }
    disposables.push_back(std::move(disposable));
}

void VgDevice::destroyDisposables() {
    for (auto& disposable : disposables) {
        disposable->destroy();
    }
    disposables.clear();
}
