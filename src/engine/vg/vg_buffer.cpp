#include "vg_buffer.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgBuffer::VgBuffer(const Config& config, VgDevice& device, const CreateInfo& createInfo) : device{&device} {

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = createInfo.memoryUsage;
    allocInfo.flags = createInfo.memoryFlags;

    VmaAllocationInfo allocationInfo;
    if (vmaCreateBuffer(device.getAllocator(), &createInfo, &allocInfo, &buffer, &allocation, &allocationInfo) !=
        VK_SUCCESS) {
        EXCEPTION("Failed to allocate buffer memory!");
    }

    bufferSize = createInfo.size;
    memoryUsage = createInfo.memoryUsage;
    mappedPtr = allocationInfo.pMappedData;
}

VgBuffer::~VgBuffer() {
    destroy();
}

VgBuffer::VgBuffer(VgBuffer&& other) noexcept {
    swap(other);
}

VgBuffer& VgBuffer::operator=(VgBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgBuffer::swap(VgBuffer& other) noexcept {
    std::swap(device, other.device);
    std::swap(buffer, other.buffer);
    std::swap(allocation, other.allocation);
    std::swap(bufferSize, other.bufferSize);
    std::swap(memoryUsage, other.memoryUsage);
    std::swap(mappedPtr, other.mappedPtr);
}

void VgBuffer::destroy() {
    if (buffer) {
        vmaDestroyBuffer(device->getAllocator(), buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }

    device = nullptr;
}

void VgBuffer::subData(const void* data, const size_t offset, const size_t size) {
    if (memoryUsage == VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY) {
        void* dst;
        if (vmaMapMemory(device->getAllocator(), allocation, &dst) != VK_SUCCESS) {
            EXCEPTION("Failed to map buffer memory!");
        }
        memcpy(reinterpret_cast<char*>(dst) + offset, data, size);
        vmaUnmapMemory(device->getAllocator(), allocation);
    } else {
        device->uploadBufferData(data, size, *this);
    }
}
