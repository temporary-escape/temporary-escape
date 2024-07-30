#pragma once

#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;
class ENGINE_API VulkanDescriptorPool;
class ENGINE_API VulkanDescriptorSetLayout;

class ENGINE_API VulkanDescriptorSet : public VulkanDisposable {
public:
    VulkanDescriptorSet() = default;
    explicit VulkanDescriptorSet(VkDevice device, VulkanDescriptorPool& descriptorPool,
                                 const VulkanDescriptorSetLayout& layout, bool autoFree = true);
    ~VulkanDescriptorSet();
    VulkanDescriptorSet(const VulkanDescriptorSet& other) = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept;
    VulkanDescriptorSet& operator=(const VulkanDescriptorSet& other) = delete;
    VulkanDescriptorSet& operator=(VulkanDescriptorSet&& other) noexcept;
    void swap(VulkanDescriptorSet& other) noexcept;
    void reset();

    void bind(const Span<VulkanBufferBinding>& uniforms = {}, const Span<VulkanTextureBinding>& textures = {},
              const Span<VulkanTextureBinding>& inputAttachments = {});
    void bind(const Span<VkWriteDescriptorSet>& writes);

    void bindUniform(uint32_t binding, const VulkanBuffer& buffer, bool dynamic = false, size_t range = 0);
    void bindTexture(uint32_t binding, const VulkanTexture& texture);
    void bindTextures(const Span<VulkanTextureBinding>& textures);

    void destroy() override;

    [[nodiscard]] VkDescriptorSet& getHandle() {
        return descriptorSet;
    }

    [[nodiscard]] const VkDescriptorSet& getHandle() const {
        return descriptorSet;
    }

    [[nodiscard]] const VulkanDescriptorSetLayout& getLayout() const {
        return *descriptorSetLayout;
    }

    operator bool() const {
        return getHandle() != VK_NULL_HANDLE;
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
    VulkanDescriptorPool* descriptorPool{nullptr};
    const VulkanDescriptorSetLayout* descriptorSetLayout{nullptr};
    bool autoFree{false};
};
} // namespace Engine
