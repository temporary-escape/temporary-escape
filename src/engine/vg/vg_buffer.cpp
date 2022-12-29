#include "vg_buffer.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgBuffer::VgBuffer(const Config& config, VgDevice& device, const CreateInfo& createInfo) :
    state{std::make_shared<BufferState>()} {

    state->device = &device;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = createInfo.memoryUsage;
    allocInfo.flags = createInfo.memoryFlags;

    VmaAllocationInfo allocationInfo;
    if (vmaCreateBuffer(device.getAllocator(), &createInfo, &allocInfo, &state->buffer, &state->allocation,
                        &allocationInfo) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate buffer memory!");
    }

    state->bufferSize = createInfo.size;
    state->memoryUsage = createInfo.memoryUsage;
    state->mappedPtr = allocationInfo.pMappedData;
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
    std::swap(state, other.state);
}

void VgBuffer::destroy() {
    if (state && state->device) {
        state->device->dispose(state);
    }

    state.reset();
}

void VgBuffer::subData(const void* data, const size_t offset, const size_t size) {
    if (state->memoryUsage == VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY ||
        state->memoryUsage == VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU) {

        void* dst;
        if (vmaMapMemory(state->device->getAllocator(), state->allocation, &dst) != VK_SUCCESS) {
            EXCEPTION("Failed to map buffer memory!");
        }
        memcpy(reinterpret_cast<char*>(dst) + offset, data, size);
        vmaUnmapMemory(state->device->getAllocator(), state->allocation);
    } else {
        state->device->uploadBufferData(data, size, *this);
    }
}

void VgBuffer::BufferState::destroy() {
    if (buffer) {
        vmaDestroyBuffer(device->getAllocator(), buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
}
