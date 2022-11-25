#include "vulkan_framebuffer.hpp"

using namespace Engine;

VulkanFramebuffer::VulkanFramebuffer(VkDevice device, VezFramebuffer handle) : device(device), handle(handle) {
}

VulkanFramebuffer::~VulkanFramebuffer() {
    reset();
}

VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept {
    swap(other);
}

VulkanFramebuffer& VulkanFramebuffer::operator=(VulkanFramebuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }

    return *this;
}

void VulkanFramebuffer::swap(VulkanFramebuffer& other) noexcept {
    auto temp = other.handle;
    other.handle = handle;
    handle = temp;
    std::swap(device, other.device);
}

void VulkanFramebuffer::reset() {
    if (handle) {
        vezDestroyFramebuffer(device, handle);
    }
    handle = VK_NULL_HANDLE;
}
