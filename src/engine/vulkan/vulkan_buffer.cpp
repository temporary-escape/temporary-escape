#include "vulkan_buffer.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanBuffer::VulkanBuffer(VulkanDevice& device, const CreateInfo& createInfo) :
    device{device.getDevice()}, allocator{device.getAllocator().getHandle()} {

    if (createInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    } else if (createInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
        descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    } else {
        descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = createInfo.memoryUsage;
    allocInfo.flags = createInfo.memoryFlags;

    VmaAllocationInfo allocationInfo;
    if (vmaCreateBuffer(allocator, &createInfo, &allocInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate buffer memory!");
    }

    bufferSize = createInfo.size;
    mappedPtr = allocationInfo.pMappedData;
}

VulkanBuffer::~VulkanBuffer() {
    destroy();
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
    std::swap(device, other.device);
    std::swap(allocator, other.allocator);
    std::swap(allocation, other.allocation);
    std::swap(buffer, other.buffer);
    std::swap(bufferSize, other.bufferSize);
    std::swap(mappedPtr, other.mappedPtr);
    std::swap(descriptorType, other.descriptorType);
}

void VulkanBuffer::destroy() {
    if (buffer) {
        vmaDestroyBuffer(allocator, buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
}

void VulkanBuffer::subDataLocal(const void* data, size_t offset, size_t size) {
    auto* dst = reinterpret_cast<char*>(mappedPtr);
    if (!mappedPtr) {
        dst = reinterpret_cast<char*>(mapMemory());
    }
    std::memcpy(dst + offset, data, size);
    if (!mappedPtr) {
        unmapMemory();
    }
    vmaFlushAllocation(allocator, allocation, offset, size);
}

void* VulkanBuffer::mapMemory() {
    void* dst;
    if (vmaMapMemory(allocator, allocation, &dst) != VK_SUCCESS) {
        EXCEPTION("Failed to map buffer memory!");
    }
    return dst;
}

void VulkanBuffer::unmapMemory() {
    vmaUnmapMemory(allocator, allocation);
}
