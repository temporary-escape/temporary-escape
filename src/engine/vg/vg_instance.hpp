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

private:
    void cleanup();

    const Config& config;
    VkInstance instance{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
};
} // namespace Engine
