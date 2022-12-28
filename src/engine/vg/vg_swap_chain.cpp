#include "vg_swap_chain.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

VgSwapChain::VgSwapChain(VkDevice device, const VkSwapchainCreateInfoKHR& createInfo) : device{device} {
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        destroy();
        EXCEPTION("failed to create swap chain!");
    }

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = createInfo.imageFormat;
    swapChainExtent = createInfo.imageExtent;

    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapChainImageFormat;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            destroy();
            EXCEPTION("failed to create image views!");
        }
    }
}

VgSwapChain::~VgSwapChain() {
    destroy();
}

void VgSwapChain::destroy() {
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    swapChainImageViews.clear();

    if (swapChain) {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }
}

VgSwapChain::VgSwapChain(VgSwapChain&& other) noexcept {
    swap(other);
}

VgSwapChain& VgSwapChain::operator=(VgSwapChain&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgSwapChain::swap(VgSwapChain& other) noexcept {
    std::swap(device, other.device);
    std::swap(swapChain, other.swapChain);
    std::swap(swapChainImageFormat, other.swapChainImageFormat);
    std::swap(swapChainExtent, other.swapChainExtent);
    std::swap(swapChainImages, other.swapChainImages);
    std::swap(swapChainImageViews, other.swapChainImageViews);
}
