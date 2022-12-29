#pragma once

#include "vg_texture.hpp"
#include "vg_types.hpp"
#include "vg_uniform_buffer.hpp"

namespace Engine {
class VgDevice;
class VgDescriptorPool;
class VgDescriptorSetLayout;

class VgDescriptorSet {
public:
    VgDescriptorSet() = default;
    explicit VgDescriptorSet(const Config& config, VgDevice& device, VgDescriptorPool& descriptorPool,
                             VgDescriptorSetLayout& layout);
    ~VgDescriptorSet();
    VgDescriptorSet(const VgDescriptorSet& other) = delete;
    VgDescriptorSet(VgDescriptorSet&& other) noexcept;
    VgDescriptorSet& operator=(const VgDescriptorSet& other) = delete;
    VgDescriptorSet& operator=(VgDescriptorSet&& other) noexcept;
    void swap(VgDescriptorSet& other) noexcept;

    void bind(const std::vector<VgUniformBufferBinding>& uniforms, const std::vector<VgTextureBinding>& textures);

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
