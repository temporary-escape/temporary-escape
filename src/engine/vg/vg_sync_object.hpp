#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgSyncObject {
public:
    VgSyncObject() = default;
    explicit VgSyncObject(const Config& config, VkDevice device);
    ~VgSyncObject();
    VgSyncObject(const VgSyncObject& other) = delete;
    VgSyncObject(VgSyncObject&& other) noexcept;
    VgSyncObject& operator=(const VgSyncObject& other) = delete;
    VgSyncObject& operator=(VgSyncObject&& other) noexcept;
    void swap(VgSyncObject& other) noexcept;

    void reset();
    void wait();

    VkSemaphore getImageAvailableSemaphore() const {
        return imageAvailableSemaphore;
    }

    VkSemaphore getRenderFinishedSemaphore() const {
        return renderFinishedSemaphore;
    }

    VkFence getHandle() const {
        return inFlightFence;
    }

    operator bool() const {
        return inFlightFence != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};
};
} // namespace Engine
