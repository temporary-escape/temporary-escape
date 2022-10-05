#include "VulkanTexture.hpp"
#include "../Utils/Exceptions.hpp"

#define CMP "VulkanTexture"

using namespace Engine;

VulkanTexture::VulkanTexture(VkDevice device, const Descriptor& desc) : device(device), desc(desc) {
    // Create the AppBase::GetDevice() side image.
    VezImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.imageType = desc.type;
    imageCreateInfo.format = desc.format;
    imageCreateInfo.extent = {static_cast<uint32_t>(desc.size.x), static_cast<uint32_t>(desc.size.y), 1};
    imageCreateInfo.mipLevels = desc.levels;
    imageCreateInfo.arrayLayers = desc.layers;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = desc.usage;
    if (vezCreateImage(device, VEZ_MEMORY_GPU_ONLY, &imageCreateInfo, &image) != VK_SUCCESS) {
        EXCEPTION("Failed to create vulkan texture of size: {} format: {} type: {}", desc.size, desc.format, desc.type);
    }

    VezImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = desc.viewType;
    imageViewCreateInfo.format = desc.format;
    imageViewCreateInfo.subresourceRange.layerCount = desc.layers;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    if (vezCreateImageView(device, &imageViewCreateInfo, &view) != VK_SUCCESS) {
        EXCEPTION("Failed to create image view for vulkan texture of size: {} format: {} type: {} ", desc.size,
                  desc.format, desc.type);
    }

    VezSamplerCreateInfo createInfo = {};
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.addressModeU = desc.addressModeU;
    createInfo.addressModeV = desc.addressModeV;
    createInfo.addressModeW = desc.addressModeW;
    if (vezCreateSampler(device, &createInfo, &sampler) != VK_SUCCESS) {
        EXCEPTION("Failed to create image sampler for vulkan texture of size: {} format: {} type: {} ", desc.size,
                  desc.format, desc.type);
    }
}

VulkanTexture::~VulkanTexture() {
    reset();
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
    std::swap(desc, other.desc);
    std::swap(image, other.image);
    std::swap(view, other.view);
    std::swap(sampler, other.sampler);
    std::swap(device, other.device);
}

void VulkanTexture::subData(int level, const Vector2i& offset, int layer, const Vector2i& size, const void* data) {
    VezImageSubDataInfo subDataInfo = {};
    subDataInfo.imageSubresource.mipLevel = level;
    subDataInfo.imageSubresource.baseArrayLayer = layer;
    subDataInfo.imageSubresource.layerCount = 1;
    subDataInfo.imageOffset = {
        static_cast<int32_t>(offset.x),
        static_cast<int32_t>(offset.y),
        0,
    };
    subDataInfo.imageExtent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    if (vezImageSubData(device, image, &subDataInfo, data) != VK_SUCCESS) {
        EXCEPTION("Failed to submit pixels for vulkan texture of size: {} offset: {} format: {} type: {}", size, offset,
                  desc.format, desc.type);
    }
}

void VulkanTexture::reset() {
    if (device && image) {
        vezDestroyImageView(device, view);
        vezDestroyImage(device, image);
        vezDestroySampler(device, sampler);
    }
    device = VK_NULL_HANDLE;
    image = VK_NULL_HANDLE;
    view = VK_NULL_HANDLE;
    sampler = VK_NULL_HANDLE;
}
