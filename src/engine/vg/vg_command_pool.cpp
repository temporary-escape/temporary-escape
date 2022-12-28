#include "vg_command_pool.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgCommandPool::VgCommandPool(const Config& config, VgDevice& device, const CreateInfo& createInfo) :
    config{&config}, device{&device} {
    if (vkCreateCommandPool(device.getHandle(), &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create command pool!");
    }
}

VgCommandPool::~VgCommandPool() {
    destroy();
}

VgCommandPool::VgCommandPool(VgCommandPool&& other) noexcept {
    swap(other);
}

VgCommandPool& VgCommandPool::operator=(VgCommandPool&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgCommandPool::swap(VgCommandPool& other) noexcept {
    std::swap(config, other.config);
    std::swap(device, other.device);
    std::swap(commandPool, other.commandPool);
}

void VgCommandPool::destroy() {
    if (commandPool) {
        vkDestroyCommandPool(device->getHandle(), commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
}

VgCommandBuffer VgCommandPool::createCommandBuffer() {
    return VgCommandBuffer{*config, *device, *this};
}
