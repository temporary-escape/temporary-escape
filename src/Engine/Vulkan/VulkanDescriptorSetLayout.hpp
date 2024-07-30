#pragma once

#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanDescriptorSetLayout : public VulkanDisposable {
public:
    using CreateInfo = VkDescriptorSetLayoutCreateInfo;
    using Binding = VkDescriptorSetLayoutBinding;

    VulkanDescriptorSetLayout() = default;
    explicit VulkanDescriptorSetLayout(VulkanDevice& device, const CreateInfo& createInfo);
    explicit VulkanDescriptorSetLayout(VulkanDevice& device, const Span<Binding>& bindings);
    ~VulkanDescriptorSetLayout();
    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other) = delete;
    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept;
    VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout& other) = delete;
    VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) noexcept;
    void swap(VulkanDescriptorSetLayout& other) noexcept;

    VkDescriptorSetLayout& getHandle() {
        return descriptorSetLayout;
    }

    const VkDescriptorSetLayout& getHandle() const {
        return descriptorSetLayout;
    }

    operator bool() const {
        return getHandle() != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
};
} // namespace Engine
