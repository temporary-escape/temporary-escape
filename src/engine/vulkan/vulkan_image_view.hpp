#pragma once

#include "../utils/path.hpp"
#include "vulkan_types.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanImageView : public VulkanDisposable {
public:
    using CreateInfo = VkImageViewCreateInfo;

    VulkanImageView() = default;
    explicit VulkanImageView(VulkanDevice& device, const CreateInfo& createInfo);
    ~VulkanImageView();
    VulkanImageView(const VulkanImageView& other) = delete;
    VulkanImageView(VulkanImageView&& other) noexcept;
    VulkanImageView& operator=(const VulkanImageView& other) = delete;
    VulkanImageView& operator=(VulkanImageView&& other) noexcept;
    void swap(VulkanImageView& other) noexcept;

    VkImageView& getHandle() {
        return view;
    }

    const VkImageView& getHandle() const {
        return view;
    }

    operator bool() const {
        return view != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
};
} // namespace Engine
