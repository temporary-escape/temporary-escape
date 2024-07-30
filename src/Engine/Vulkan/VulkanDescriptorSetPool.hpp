#pragma once

#include "VulkanDescriptorPool.hpp"
#include "VulkanDescriptorSetLayout.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanDescriptorSetPool : public VulkanDisposable {
public:
    using Binding = VkDescriptorSetLayoutBinding;

    VulkanDescriptorSetPool() = default;
    explicit VulkanDescriptorSetPool(VulkanDevice& device, const Span<Binding>& bindings, size_t maxSets);
    ~VulkanDescriptorSetPool();
    VulkanDescriptorSetPool(const VulkanDescriptorSetPool& other) = delete;
    VulkanDescriptorSetPool(VulkanDescriptorSetPool&& other) noexcept;
    VulkanDescriptorSetPool& operator=(const VulkanDescriptorSetPool& other) = delete;
    VulkanDescriptorSetPool& operator=(VulkanDescriptorSetPool&& other) noexcept;
    void swap(VulkanDescriptorSetPool& other) noexcept;

    operator bool() const {
        return pool && layout;
    }

    void destroy() override;

    VulkanDescriptorSet createDescriptorSet();

private:
    VulkanDevice* device{nullptr};
    VulkanDescriptorPool pool;
    VulkanDescriptorSetLayout layout;
};
} // namespace Engine
