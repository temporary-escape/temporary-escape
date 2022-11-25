#include "vulkan_buffer.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

VulkanBuffer::VulkanBuffer(VkDevice device, Type type, Usage usage, size_t size) : device{device}, size{size} {
    VezBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | static_cast<VkBufferUsageFlags>(type);

    const auto flags = static_cast<VezMemoryFlags>(usage);

    if (vezCreateBuffer(device, flags, &bufferCreateInfo, &buffer) != VK_SUCCESS) {
        EXCEPTION("vezCreateBuffer failed to create vertex buffer of size {}", size);
    }
}

VulkanBuffer::~VulkanBuffer() {
    reset();
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept {
    swap(other);
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }

    return *this;
}

void VulkanBuffer::swap(VulkanBuffer& other) noexcept {
    std::swap(buffer, other.buffer);
    std::swap(device, other.device);
    std::swap(size, other.size);
}

void VulkanBuffer::subData(const void* data, const size_t offset, const size_t size) {
    if (buffer == VK_NULL_HANDLE) {
        EXCEPTION("vezBufferSubData failed to create vertex buffer");
    }

    if (vezBufferSubData(device, buffer, 0, size, data) != VK_SUCCESS) {
        EXCEPTION("vezBufferSubData failed to set vertex buffer of size {}", size);
    }
}

void VulkanBuffer::reset() {
    if (device && buffer) {
        vezDestroyBuffer(device, buffer);
    }
    device = VK_NULL_HANDLE;
    buffer = VK_NULL_HANDLE;
}

void* VulkanBuffer::mapPtr(const size_t size) {
    void* data = nullptr;
    auto result = vezMapBuffer(device, buffer, 0, size, &data);
    if (result != VK_SUCCESS) {
        EXCEPTION("vezMapBuffer failed");
    }

    return data;
}

void VulkanBuffer::unmap() {
    vezUnmapBuffer(device, buffer);
}
