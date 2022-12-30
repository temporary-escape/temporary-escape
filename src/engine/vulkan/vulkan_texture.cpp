#include "vulkan_texture.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanTexture::VulkanTexture(VulkanDevice& device, const CreateInfo& createInfo) :
    device{device.getDevice()}, allocator{device.getAllocator().getHandle()} {

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.flags = 0;

    VmaAllocationInfo allocationInfo;
    if (vmaCreateImage(allocator, &createInfo.image, &allocInfo, &image, &allocation, &allocationInfo) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to allocate image memory!");
    }

    if (vkCreateSampler(device.getDevice(), &createInfo.sampler, nullptr, &sampler) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to allocate image sampler!");
    }

    auto createInfoView = createInfo.view;
    createInfoView.image = image;

    if (vkCreateImageView(device.getDevice(), &createInfoView, nullptr, &view) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate image view!");
    }

    format = createInfo.image.format;
    extent = createInfo.image.extent;
}

VulkanTexture::~VulkanTexture() {
    destroy();
}

VulkanTexture::VulkanTexture(VulkanTexture&& other) noexcept {
    swap(other);
}

VulkanTexture& VulkanTexture::operator=(VulkanTexture&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanTexture::swap(VulkanTexture& other) noexcept {
    std::swap(device, other.device);
    std::swap(allocator, other.allocator);
    std::swap(allocation, other.allocation);
    std::swap(image, other.image);
    std::swap(view, other.view);
    std::swap(sampler, other.sampler);
    std::swap(format, other.format);
    std::swap(extent, other.extent);
}

void VulkanTexture::destroy() {
    if (sampler) {
        vkDestroySampler(device, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }

    if (view) {
        vkDestroyImageView(device, view, nullptr);
        view = VK_NULL_HANDLE;
    }

    if (image) {
        vmaDestroyImage(allocator, image, allocation);
        image = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
}

VkDeviceSize VulkanTexture::getDataSize() const {
    return getFormatDataSize(format, extent);
}
