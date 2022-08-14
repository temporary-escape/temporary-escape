#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "VEZ/VEZ.h"
#include <type_traits>

namespace Engine {
class ENGINE_API VulkanTexture {
public:
    using Format = VkFormat;
    using Type = VkImageType;
    using Usage = VkImageUsageFlagBits;
    using ViewType = VkImageViewType;

    struct Descriptor {
        Type type{Type::VK_IMAGE_TYPE_2D};
        Format format{Format::VK_FORMAT_R8G8B8A8_UNORM};
        VkImageUsageFlags usage{Usage::VK_IMAGE_USAGE_SAMPLED_BIT | Usage::VK_IMAGE_USAGE_TRANSFER_DST_BIT};
        Vector2i size;
        ViewType viewType{ViewType::VK_IMAGE_VIEW_TYPE_2D};
        int levels{1};
        int layers{1};
    };

    NON_COPYABLE(VulkanTexture);

    VulkanTexture() = default;
    explicit VulkanTexture(VkDevice device, const Descriptor& desc);
    ~VulkanTexture();
    VulkanTexture(VulkanTexture&& other) noexcept;
    VulkanTexture& operator=(VulkanTexture&& other) noexcept;
    void swap(VulkanTexture& other) noexcept;

    void subData(int level, const Vector2i& offset, const Vector2i& size, const void* data) {
        subData(level, offset, 0, size, data);
    }
    void subData(int level, const Vector2i& offset, int layer, const Vector2i& size, const void* data);
    void reset();

    [[nodiscard]] VkImage& getHandle() {
        return image;
    }

    [[nodiscard]] const VkImage& getHandle() const {
        return image;
    }

    [[nodiscard]] VkImageView& getView() {
        return view;
    }

    [[nodiscard]] const VkImageView& getView() const {
        return view;
    }

    [[nodiscard]] VkSampler& getSampler() {
        return sampler;
    }

    [[nodiscard]] const VkSampler& getSampler() const {
        return sampler;
    }

    [[nodiscard]] operator bool() const {
        return image != VK_NULL_HANDLE;
    }

private:
    Descriptor desc;
    VkDevice device{VK_NULL_HANDLE};
    VkImage image{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
};

static_assert(std::is_move_constructible<VulkanTexture>::value, "VulkanTexture must be move constructible");
static_assert(std::is_move_assignable<VulkanTexture>::value, "VulkanTexture must be move assignable");
} // namespace Engine
