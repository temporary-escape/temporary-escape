#pragma once

#include "vg_types.hpp"
#include <vector>

namespace Engine {
class VgDevice;

class VgSwapChain {
public:
    VgSwapChain() = default;
    explicit VgSwapChain(VgDevice& device, const VkSwapchainCreateInfoKHR& createInfo);
    ~VgSwapChain();
    VgSwapChain(const VgSwapChain& other) = delete;
    VgSwapChain(VgSwapChain&& other) noexcept;
    VgSwapChain& operator=(const VgSwapChain& other) = delete;
    VgSwapChain& operator=(VgSwapChain&& other) noexcept;
    void swap(VgSwapChain& other) noexcept;

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
    VgDevice* device{nullptr};
    VkSwapchainKHR swapChain{VK_NULL_HANDLE};
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
};
} // namespace Engine
