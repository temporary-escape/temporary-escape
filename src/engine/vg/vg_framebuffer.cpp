#include "vg_framebuffer.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgFramebuffer::VgFramebuffer(const Config& config, VgDevice& device, const CreateInfo& createInfo) : device{&device} {
    if (vkCreateFramebuffer(device.getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        EXCEPTION("Failed to create framebuffer!");
    }
}

VgFramebuffer::~VgFramebuffer() {
    destroy();
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

void VgFramebuffer::destroy() {
    if (framebuffer) {
        vkDestroyFramebuffer(device->getDevice(), framebuffer, nullptr);
        framebuffer = VK_NULL_HANDLE;
    }
}
