#pragma once

#include "vulkan_types.hpp"
#include <vector>

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanSwapChain {
public:
    VulkanSwapChain() = default;
    explicit VulkanSwapChain(VulkanDevice& device, const VkSwapchainCreateInfoKHR& createInfo);
    ~VulkanSwapChain();
    VulkanSwapChain(const VulkanSwapChain& other) = delete;
    VulkanSwapChain(VulkanSwapChain&& other) noexcept;
    VulkanSwapChain& operator=(const VulkanSwapChain& other) = delete;
    VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;
    void swap(VulkanSwapChain& other) noexcept;

    VkSwapchainKHR& getHandle() {
        return swapChain;
    }

    const VkSwapchainKHR& getHandle() const {
        return swapChain;
    }

    VkFormat getFormat() const {
        return swapChainImageFormat;
    }

    const std::vector<VkImage>& getImages() const {
        return swapChainImages;
    }

    const std::vector<VkImageView>& getImageViews() const {
        return swapChainImageViews;
    }

    const VkExtent2D& getExtent() const {
        return swapChainExtent;
    }

    operator bool() const {
        return swapChain != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VkSwapchainKHR swapChain{VK_NULL_HANDLE};
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
};
} // namespace Engine
