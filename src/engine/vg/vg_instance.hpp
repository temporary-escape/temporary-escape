#pragma once

#include "vg_window.hpp"

namespace Engine {
class ENGINE_API VgInstance : public VgWindow {
public:
    explicit VgInstance(const Config& config);
    virtual ~VgInstance();

    VgQueueFamilyIndices getQueueFamilies();
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
