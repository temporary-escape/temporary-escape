#pragma once

#include "../utils/path.hpp"
#include "vulkan_types.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanTexture : public VulkanDisposable {
public:
    struct CreateInfo {
        CreateInfo() {
            image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image.imageType = VK_IMAGE_TYPE_2D;
            image.extent.depth = 1;
            image.mipLevels = 1;
            image.arrayLayers = 1;
            image.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
            image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image.samples = VK_SAMPLE_COUNT_1_BIT;
            image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view.subresourceRange.baseMipLevel = 0;
            view.subresourceRange.levelCount = 1;
            view.subresourceRange.baseArrayLayer = 0;
            view.subresourceRange.layerCount = 1;

            sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler.magFilter = VK_FILTER_LINEAR;
            sampler.minFilter = VK_FILTER_LINEAR;
            sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler.anisotropyEnable = VK_FALSE;
            sampler.maxAnisotropy = 1.0f;
            sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            sampler.unnormalizedCoordinates = VK_FALSE;
            sampler.compareEnable = VK_FALSE;
            sampler.compareOp = VK_COMPARE_OP_ALWAYS;
            sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }

        VkImageCreateInfo image{};
        VkImageViewCreateInfo view{};
        VkSamplerCreateInfo sampler{};
    };

    VulkanTexture() = default;
    explicit VulkanTexture(VulkanDevice& device, const CreateInfo& createInfo);
    ~VulkanTexture();
    VulkanTexture(const VulkanTexture& other) = delete;
    VulkanTexture(VulkanTexture&& other) noexcept;
    VulkanTexture& operator=(const VulkanTexture& other) = delete;
    VulkanTexture& operator=(VulkanTexture&& other) noexcept;
    void swap(VulkanTexture& other) noexcept;

    // void subData(int level, const Vector2i& offset, int layer, const Vector2i& size, const void* data);
    // void finalize();

    VkImage& getHandle() {
        return image;
    }

    const VkImage& getHandle() const {
        return image;
    }

    VkImageView& getImageView() {
        return view;
    }

    const VkImageView& getImageView() const {
        return view;
    }

    VkSampler& getSampler() {
        return sampler;
    }

    const VkSampler& getSampler() const {
        return sampler;
    }

    VmaAllocation getAllocation() const {
        return allocation;
    }

    VkFormat getFormat() const {
        return format;
    }

    VkDeviceSize getDataSize() const;

    const VkExtent3D& getExtent() const {
        return extent;
    }

    uint32_t getMipMaps() const {
        return mipMaps;
    }

    operator bool() const {
        return image != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkImage image{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
    VkFormat format{VK_FORMAT_UNDEFINED};
    VkExtent3D extent{0, 0, 0};
    uint32_t mipMaps{0};
};

struct ENGINE_API VulkanTextureBinding {
    uint32_t binding{0};
    const VulkanTexture* texture{nullptr};
};

inline uint32_t getMipMapLevels(const Vector2i& size) {
    return static_cast<uint32_t>(std::floor(std::log2(std::max(size.x, size.y)))) + 1;
}
} // namespace Engine
