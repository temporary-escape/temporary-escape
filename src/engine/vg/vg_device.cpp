#include "vg_device.hpp"
#include "../utils/exceptions.hpp"

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

    if (config.enableValidationLayers) {
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

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        cleanup();
        EXCEPTION("Failed to create command pool!");
    }

    syncObject = createSyncObject();
}

VgDevice::~VgDevice() {
    cleanup();
}

void VgDevice::cleanup() {
    if (commandPool) {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }

    swapChain = VgSwapChain{};
    syncObject = VgSyncObject{};

    if (device) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
}

void VgDevice::onNextFrame() {
    syncObject.wait();

    const auto result =
        vkAcquireNextImageKHR(device, swapChain.getHandle(), UINT64_MAX, syncObject.getImageAvailableSemaphore(),
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
}

void VgDevice::onExit() {
    waitDeviceIdle();
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

VgCommandBuffer VgDevice::createCommandBuffer() {
    return VgCommandBuffer{config, device, commandPool};
}

void VgDevice::waitDeviceIdle() {
    vkDeviceWaitIdle(device);
}

void VgDevice::submitCommandBuffer(const VgCommandBuffer& commandBuffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {syncObject.getImageAvailableSemaphore()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    const auto commandBufferHandle = commandBuffer.getHandle();

    VkSemaphore signalSemaphores[] = {syncObject.getRenderFinishedSemaphore()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBufferHandle;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, syncObject.getHandle()) != VK_SUCCESS) {
        EXCEPTION("Failed to submit draw command buffer!");
    }
}

void VgDevice::submitPresentQueue() {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = {syncObject.getRenderFinishedSemaphore()};

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
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        EXCEPTION("Failed to present swap chain image!");
    }
}

void VgDevice::recreateSwapChain() {
    waitUntilValidFramebufferSize();

    waitDeviceIdle();

    swapChain = VgSwapChain{};
    swapChain = VgSwapChain(device, getSwapChainInfo());

    onSwapChainChanged();
}
