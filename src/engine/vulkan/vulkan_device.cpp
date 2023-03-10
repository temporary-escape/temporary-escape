#include "vulkan_device.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

VulkanDevice::VulkanDevice(const Config& config) : VulkanInstance{config}, config{config} {
    const auto indices = getQueueFamilies();

    logger.info("Queue family graphics: {}", indices.graphicsFamily ? indices.graphicsFamily.value() : -1);
    logger.info("Queue family present: {}", indices.presentFamily ? indices.presentFamily.value() : -1);
    logger.info("Queue family compute: {}", indices.computeFamily ? indices.computeFamily.value() : -1);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value(),
        indices.computeFamily.value(),
    };

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
    deviceFeatures.geometryShader = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = vulkanDeviceExtensions.data();

    if (config.vulkan.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(vulkanValidationLayers.size());
        createInfo.ppEnabledLayerNames = vulkanValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(getPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS) {
        destroy();
        EXCEPTION("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);

    allocator = VulkanAllocator{*this};
}

VulkanDevice::~VulkanDevice() {
    destroy();
}

void VulkanDevice::destroy() {
    allocator.destroy();

    if (device) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
}
