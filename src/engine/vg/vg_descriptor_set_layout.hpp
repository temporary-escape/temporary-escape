#pragma once

#include "vg_types.hpp"

namespace Engine {
class VgRenderer;

class VgDescriptorSetLayout {
public:
    using CreateInfo = VkDescriptorSetLayoutCreateInfo;

    VgDescriptorSetLayout() = default;
    explicit VgDescriptorSetLayout(const Config& config, VgRenderer& device, const CreateInfo& createInfo);
    ~VgDescriptorSetLayout();
    VgDescriptorSetLayout(const VgDescriptorSetLayout& other) = delete;
    VgDescriptorSetLayout(VgDescriptorSetLayout&& other) noexcept;
    VgDescriptorSetLayout& operator=(const VgDescriptorSetLayout& other) = delete;
    VgDescriptorSetLayout& operator=(VgDescriptorSetLayout&& other) noexcept;
    void swap(VgDescriptorSetLayout& other) noexcept;

    VkDescriptorSetLayout& getHandle() {
        return state->descriptorSetLayout;
    }

    const VkDescriptorSetLayout& getHandle() const {
        return state->descriptorSetLayout;
    }

    operator bool() const {
        return getHandle() != VK_NULL_HANDLE;
    }

    void destroy();

private:
    class DescriptionSetLayoutState : public VgDisposable {
    public:
        void destroy() override;

        VgRenderer* device{nullptr};
        VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    };
    std::shared_ptr<DescriptionSetLayoutState> state;
};
} // namespace Engine
