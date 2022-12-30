#include "vulkan_double_buffer.hpp"
#include "vulkan_renderer.hpp"

using namespace Engine;

VulkanDoubleBuffer::VulkanDoubleBuffer(VulkanRenderer& device, const CreateInfo& createInfo) : device{&device} {
    for (auto& buffer : buffers) {
        buffer = device.createBuffer(createInfo);
    }
}

VulkanDoubleBuffer::~VulkanDoubleBuffer() {
    destroy();
}

VulkanDoubleBuffer::VulkanDoubleBuffer(VulkanDoubleBuffer&& other) noexcept {
    swap(other);
}

VulkanDoubleBuffer& VulkanDoubleBuffer::operator=(VulkanDoubleBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanDoubleBuffer::swap(VulkanDoubleBuffer& other) noexcept {
    std::swap(device, other.device);
    std::swap(buffers, other.buffers);
}

void VulkanDoubleBuffer::destroy() {
    for (auto& buffer : buffers) {
        buffer.destroy();
    }
    device = nullptr;
}

void VulkanDoubleBuffer::subDataLocal(const void* data, size_t offset, size_t size) {
    getCurrentBuffer().subDataLocal(data, offset, size);
}

void* VulkanDoubleBuffer::mapMemory() {
    return getCurrentBuffer().mapMemory();
}

void VulkanDoubleBuffer::unmapMemory() {
    getCurrentBuffer().unmapMemory();
}

const VulkanBuffer& VulkanDoubleBuffer::getCurrentBuffer() const {
    if (buffers.size() == 1) {
        return buffers.back();
    }
    return buffers.at(device->getCurrentFrameNum());
}

VulkanBuffer& VulkanDoubleBuffer::getCurrentBuffer() {
    if (buffers.size() == 1) {
        return buffers.back();
    }
    return buffers.at(device->getCurrentFrameNum());
}
