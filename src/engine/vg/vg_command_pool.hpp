#pragma once

#include "vg_command_buffer.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgCommandPool {
public:
    using CreateInfo = VkCommandPoolCreateInfo;

    VgCommandPool() = default;
    explicit VgCommandPool(const Config& config, VgDevice& device, const CreateInfo& createInfo);
    ~VgCommandPool();
    VgCommandPool(const VgCommandPool& other) = delete;
    VgCommandPool(VgCommandPool&& other) noexcept;
    VgCommandPool& operator=(const VgCommandPool& other) = delete;
    VgCommandPool& operator=(VgCommandPool&& other) noexcept;
    void swap(VgCommandPool& other) noexcept;

    VgCommandBuffer createCommandBuffer();

    VkCommandPool& getHandle() {
        return commandPool;
    }

    const VkCommandPool& getHandle() const {
        return commandPool;
    }

    operator bool() const {
        return getHandle() != VK_NULL_HANDLE;
    }

    void destroy();

private:
    const Config* config{nullptr};
    VgDevice* device{nullptr};
    VkCommandPool commandPool{VK_NULL_HANDLE};
};
} // namespace Engine
