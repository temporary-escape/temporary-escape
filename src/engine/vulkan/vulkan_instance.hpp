#pragma once

#include "vulkan_window.hpp"

namespace Engine {
class ENGINE_API VulkanInstance : public VulkanWindow {
public:
    explicit VulkanInstance(const Config& config);
    virtual ~VulkanInstance();

    VulkanQueueFamilyIndices getQueueFamilies();
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const {
        return physicalDevice;
    }
    VkSwapchainCreateInfoKHR getSwapChainInfo();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    [[nodiscard]] VkInstance getInstance() const {
        return instance;
    }

    const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const {
        return physicalDeviceProperties;
    }

    VkFormatProperties getPhysicalDeviceFormatProperties(VkFormat format);

private:
    void destroy();

    const Config& config;
    VkInstance instance{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties physicalDeviceProperties;
};
} // namespace Engine