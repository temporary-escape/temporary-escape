#pragma once

#include "../library.hpp"
#include "vez/VEZ.h"
#include "vulkan_texture.hpp"
#include <type_traits>

namespace Engine {
struct VulkanFramebufferAttachmentReference {
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clearValue;
};

struct VulkanFramebufferAttachment {
    std::reference_wrapper<VulkanTexture> texture;
};

class VulkanFramebuffer {
public:
    NON_COPYABLE(VulkanFramebuffer);

    VulkanFramebuffer() = default;
    explicit VulkanFramebuffer(VkDevice device, VezFramebuffer handle);
    ~VulkanFramebuffer();
    VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
    VulkanFramebuffer& operator=(VulkanFramebuffer&& other) noexcept;
    void swap(VulkanFramebuffer& other) noexcept;

    void reset();

    [[nodiscard]] VezFramebuffer& getHandle() {
        return handle;
    }

    [[nodiscard]] const VezFramebuffer& getHandle() const {
        return handle;
    }

    [[nodiscard]] operator bool() const {
        return handle != VK_NULL_HANDLE;
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VezFramebuffer handle{VK_NULL_HANDLE};
};

static_assert(std::is_move_constructible<VulkanFramebuffer>::value, "VulkanFramebuffer must be move constructible");
static_assert(std::is_move_assignable<VulkanFramebuffer>::value, "VulkanFramebuffer must be move assignable");
} // namespace Engine
