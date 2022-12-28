#include "vg_framebuffer.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

VgFramebuffer::VgFramebuffer(const Config& config, VkDevice device, const CreateInfo& createInfo) : device{device} {
    if (vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to create framebuffer!");
    }
}

VgFramebuffer::~VgFramebuffer() {
    cleanup();
}

VgFramebuffer::VgFramebuffer(VgFramebuffer&& other) noexcept {
    swap(other);
}

VgFramebuffer& VgFramebuffer::operator=(VgFramebuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgFramebuffer::swap(VgFramebuffer& other) noexcept {
    std::swap(device, other.device);
    std::swap(framebuffer, other.framebuffer);
}

void VgFramebuffer::cleanup() {
    if (framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
        framebuffer = VK_NULL_HANDLE;
    }
}
