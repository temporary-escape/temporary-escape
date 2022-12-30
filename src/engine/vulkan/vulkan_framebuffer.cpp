#include "vulkan_framebuffer.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& device, const CreateInfo& createInfo) : device{device.getDevice()} {
    if (vkCreateFramebuffer(device.getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to create framebuffer!");
    }
}

VulkanFramebuffer::~VulkanFramebuffer() {
    destroy();
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
    std::swap(device, other.device);
    std::swap(framebuffer, other.framebuffer);
}

void VulkanFramebuffer::destroy() {
    if (framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
        framebuffer = VK_NULL_HANDLE;
    }
}
