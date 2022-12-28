#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgFramebuffer {
public:
    using CreateInfo = VkFramebufferCreateInfo;

    VgFramebuffer() = default;
    explicit VgFramebuffer(const Config& config, VkDevice device, const CreateInfo& createInfo);
    ~VgFramebuffer();
    VgFramebuffer(const VgFramebuffer& other) = delete;
    VgFramebuffer(VgFramebuffer&& other) noexcept;
    VgFramebuffer& operator=(const VgFramebuffer& other) = delete;
    VgFramebuffer& operator=(VgFramebuffer&& other) noexcept;
    void swap(VgFramebuffer& other) noexcept;

    VkFramebuffer getHandle() const {
        return framebuffer;
    }

    operator bool() const {
        return framebuffer != VK_NULL_HANDLE;
    }

private:
    void cleanup();

    VkDevice device{VK_NULL_HANDLE};
    VkFramebuffer framebuffer{VK_NULL_HANDLE};
};
} // namespace Engine
