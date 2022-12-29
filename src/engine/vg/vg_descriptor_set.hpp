#pragma once

#include "vg_types.hpp"

namespace Engine {
class VgDevice;
class VgDescriptorPool;
class VgDescriptorSetLayout;

class VgDescriptorSet {
public:
    struct CreateInfo {};

    VgDescriptorSet() = default;
    explicit VgDescriptorSet(const Config& config, VgDevice& device, VgDescriptorPool& descriptorPool,
                             VgDescriptorSetLayout& layout);
    ~VgDescriptorSet();
    VgDescriptorSet(const VgDescriptorSet& other) = delete;
    VgDescriptorSet(VgDescriptorSet&& other) noexcept;
    VgDescriptorSet& operator=(const VgDescriptorSet& other) = delete;
    VgDescriptorSet& operator=(VgDescriptorSet&& other) noexcept;
    void swap(VgDescriptorSet& other) noexcept;

    VkDescriptorSet getHandle() const {
        return state->descriptorSet;
    }

    operator bool() const {
        return getHandle() != VK_NULL_HANDLE;
    }

    void destroy();

private:
    class DescriptionSetState : public VgDisposable {
    public:
        void destroy() override;

        VgDevice* device{nullptr};
        VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
    };
    std::shared_ptr<DescriptionSetState> state;
};
} // namespace Engine
