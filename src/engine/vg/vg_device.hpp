#pragma once

#include "vg_instance.hpp"

namespace Engine {
class ENGINE_API VgDevice : public VgInstance {
public:
    explicit VgDevice(const Config& config);
    virtual ~VgDevice();

    VkDevice& getDevice() {
        return device;
    }

    const VkDevice& getDevice() const {
        return device;
    }

    VkQueue getGraphicsQueue() const {
        return graphicsQueue;
    }

    VkQueue getPresentQueue() const {
        return presentQueue;
    }

private:
    void destroy();

    const Config& config;
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
};
} // namespace Engine
