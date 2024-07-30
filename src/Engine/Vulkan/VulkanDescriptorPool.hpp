#pragma once

#include "VulkanDescriptorSet.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanDescriptorPool : public VulkanDisposable {
public:
    using CreateInfo = VkDescriptorPoolCreateInfo;
    using Binding = VkDescriptorSetLayoutBinding;

    VulkanDescriptorPool() = default;
    explicit VulkanDescriptorPool(VulkanDevice& device);
    explicit VulkanDescriptorPool(VulkanDevice& device, const CreateInfo& createInfo);
    explicit VulkanDescriptorPool(VulkanDevice& device, const Span<Binding>& bindings, const size_t maxSets);
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

    operator bool() const {
        return descriptorPool != VK_NULL_HANDLE;
    }

    VulkanDescriptorSet createDescriptorSet(const VulkanDescriptorSetLayout& layout) {
        return VulkanDescriptorSet{device, *this, layout, false};
    }

    void setAllocated(size_t value);
    void setFreed(size_t value);
    [[nodiscard]] size_t getFree() const {
        if (count > maxSets) {
            return 0;
        }
        return maxSets - count;
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
    size_t count{0};
    size_t maxSets{0};
};
} // namespace Engine
