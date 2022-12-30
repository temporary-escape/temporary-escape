#pragma once

#include "vulkan_descriptor_set.hpp"
#include "vulkan_types.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanDescriptorPool : public VulkanDisposable {
public:
    VulkanDescriptorPool() = default;
    explicit VulkanDescriptorPool(VulkanDevice& device);
    ~VulkanDescriptorPool();
    VulkanDescriptorPool(const VulkanDescriptorPool& other) = delete;
    VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept;
    VulkanDescriptorPool& operator=(const VulkanDescriptorPool& other) = delete;
    VulkanDescriptorPool& operator=(VulkanDescriptorPool&& other) noexcept;
    void swap(VulkanDescriptorPool& other) noexcept;

    void reset();

    void destroy() override;

    VkDescriptorPool& getHandle() {
        return descriptorPool;
    }

    const VkDescriptorPool& getHandle() const {
        return descriptorPool;
    }

    VulkanDescriptorSet createDescriptorSet(VulkanDescriptorSetLayout& layout) {
        return VulkanDescriptorSet{device, *this, layout};
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
};
} // namespace Engine
