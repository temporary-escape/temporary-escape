#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgRenderer;

class VgTexture {
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

    VgTexture() = default;
    explicit VgTexture(const Config& config, VgRenderer& device, const CreateInfo& createInfo);
    ~VgTexture();
    VgTexture(const VgTexture& other) = delete;
    VgTexture(VgTexture&& other) noexcept;
    VgTexture& operator=(const VgTexture& other) = delete;
    VgTexture& operator=(VgTexture&& other) noexcept;
    void swap(VgTexture& other) noexcept;

    void subData(int level, const Vector2i& offset, int layer, const Vector2i& size, const void* data);

    VkImage& getHandle() {
        return state->image;
    }

    const VkImage& getHandle() const {
        return state->image;
    }

    VkImageView& getImageView() {
        return state->view;
    }

    const VkImageView& getImageView() const {
        return state->view;
    }

    VkSampler& getSampler() {
        return state->sampler;
    }

    const VkSampler& getSampler() const {
        return state->sampler;
    }

    VmaAllocation getAllocation() const {
        return state->allocation;
    }

    VkFormat getFormat() const {
        return state->format;
    }

    VkDeviceSize getDataSize() const;

    operator bool() const {
        return state->image != VK_NULL_HANDLE;
    }

    void destroy();

private:
    class BufferState : public VgDisposable {
    public:
        void destroy() override;

        VgRenderer* device{nullptr};
        VmaAllocation allocation{VK_NULL_HANDLE};
        VkImage image{VK_NULL_HANDLE};
        VkImageView view{VK_NULL_HANDLE};
        VkSampler sampler{VK_NULL_HANDLE};
        VkFormat format{VK_FORMAT_UNDEFINED};
        VkExtent3D extent{0, 0};
    };

    std::shared_ptr<BufferState> state;
};

struct VgTextureBinding {
    uint32_t binding{0};
    VgTexture* texture{nullptr};
};
} // namespace Engine
