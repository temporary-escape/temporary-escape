#include "VulkanTexture.hpp"
#include "../Utils/Exceptions.hpp"
#include "VulkanDevice.hpp"

using namespace Engine;

VulkanTexture::VulkanTexture(VulkanDevice& device, const CreateInfo& createInfo) :
    device{device.getDevice()}, allocator{device.getAllocator().getHandle()} {

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

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
    mipMaps = createInfo.image.mipLevels;
    layerCount = createInfo.image.arrayLayers;
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
    std::swap(mipMaps, other.mipMaps);
    std::swap(layerCount, other.layerCount);
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
