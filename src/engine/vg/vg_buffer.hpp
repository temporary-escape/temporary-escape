#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgDevice;

class VgBuffer {
public:
    struct CreateInfo : VkBufferCreateInfo {
        VmaMemoryUsage memoryUsage{VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY};
        VmaAllocationCreateFlags memoryFlags{0};
    };

    VgBuffer() = default;
    explicit VgBuffer(const Config& config, VgDevice& device, const CreateInfo& createInfo);
    ~VgBuffer();
    VgBuffer(const VgBuffer& other) = delete;
    VgBuffer(VgBuffer&& other) noexcept;
    VgBuffer& operator=(const VgBuffer& other) = delete;
    VgBuffer& operator=(VgBuffer&& other) noexcept;
    void swap(VgBuffer& other) noexcept;

    void subData(const void* data, size_t offset, size_t size);

    VkBuffer& getHandle() {
        return state->buffer;
    }

    const VkBuffer& getHandle() const {
        return state->buffer;
    }

    VmaAllocation getAllocation() const {
        return state->allocation;
    }

    void* getMappedPtr() const {
        return state->mappedPtr;
    }

    VkDeviceSize getSize() const {
        return state->bufferSize;
    }

    operator bool() const {
        return state->buffer != VK_NULL_HANDLE;
    }

    void destroy();

private:
    class BufferState : public VgDisposable {
    public:
        void destroy() override;

        VgDevice* device{nullptr};
        VkBuffer buffer{VK_NULL_HANDLE};
        VmaAllocation allocation{VK_NULL_HANDLE};
        VkDeviceSize bufferSize{0};
        void* mappedPtr{nullptr};
        VmaMemoryUsage memoryUsage;
    };

    std::shared_ptr<BufferState> state;
};
} // namespace Engine
