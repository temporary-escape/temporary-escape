#include "vulkan_image_view.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanImageView::VulkanImageView(VulkanDevice& device, const CreateInfo& createInfo) : device{device.getDevice()} {

    if (vkCreateImageView(device.getDevice(), &createInfo, nullptr, &view) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate image view!");
    }
}

VulkanImageView::~VulkanImageView() {
    VulkanImageView::destroy();
}

VulkanImageView::VulkanImageView(VulkanImageView&& other) noexcept {
    swap(other);
}

VulkanImageView& VulkanImageView::operator=(VulkanImageView&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanImageView::swap(VulkanImageView& other) noexcept {
    std::swap(device, other.device);
    std::swap(view, other.view);
}

void VulkanImageView::destroy() {
    if (view) {
        vkDestroyImageView(device, view, nullptr);
        view = VK_NULL_HANDLE;
    }
}
