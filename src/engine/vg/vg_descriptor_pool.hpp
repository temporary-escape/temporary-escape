#pragma once

#include "vg_types.hpp"

namespace Engine {
class VgDevice;

class VgDescriptorPool {
public:
    VgDescriptorPool() = default;
    explicit VgDescriptorPool(const Config& config, VgDevice& device);
    ~VgDescriptorPool();
    VgDescriptorPool(const VgDescriptorPool& other) = delete;
    VgDescriptorPool(VgDescriptorPool&& other) noexcept;
    VgDescriptorPool& operator=(const VgDescriptorPool& other) = delete;
    VgDescriptorPool& operator=(VgDescriptorPool&& other) noexcept;
    void swap(VgDescriptorPool& other) noexcept;

    void reset();

    void destroy();

    VkDescriptorPool& getHandle() {
        return descriptorPool;
    }

    const VkDescriptorPool& getHandle() const {
        return descriptorPool;
    }

private:
    const Config* config{nullptr};
    VgDevice* device{nullptr};
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
};
} // namespace Engine