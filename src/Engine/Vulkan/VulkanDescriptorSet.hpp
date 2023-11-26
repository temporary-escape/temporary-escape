#pragma once

#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;
class ENGINE_API VulkanDescriptorPool;
class ENGINE_API VulkanDescriptorSetLayout;

class ENGINE_API VulkanDescriptorSet {
public:
    VulkanDescriptorSet() = default;
    explicit VulkanDescriptorSet(VkDevice device, const VulkanDescriptorPool& descriptorPool,
                                 const VulkanDescriptorSetLayout& layout);
    ~VulkanDescriptorSet();
    VulkanDescriptorSet(const VulkanDescriptorSet& other) = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept;
    VulkanDescriptorSet& operator=(const VulkanDescriptorSet& other) = delete;
    VulkanDescriptorSet& operator=(VulkanDescriptorSet&& other) noexcept;
    void swap(VulkanDescriptorSet& other) noexcept;

    void bind(const Span<VulkanBufferBinding>& uniforms = {}, const Span<VulkanTextureBinding>& textures = {},
              const Span<VulkanTextureBinding>& inputAttachments = {});
    void bind(const Span<VkWriteDescriptorSet>& writes);

    VkDescriptorSet& getHandle() {
        return descriptorSet;
    }

    const VkDescriptorSet& getHandle() const {
        return descriptorSet;
    }

    operator bool() const {
        return getHandle() != VK_NULL_HANDLE;
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
};
} // namespace Engine
