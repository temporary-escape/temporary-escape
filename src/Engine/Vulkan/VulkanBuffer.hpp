#pragma once

#include "../Utils/Path.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;
class ENGINE_API VulkanAllocator;

class ENGINE_API VulkanBuffer : public VulkanDisposable {
public:
    struct CreateInfo : VkBufferCreateInfo {
        VmaMemoryUsage memoryUsage{VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY};
        VmaAllocationCreateFlags memoryFlags{0};
        VkMemoryPropertyFlags memoryRequiredFlags{0};
        VkMemoryPropertyFlags memoryPreferredFlags{0};
    };

    VulkanBuffer() = default;
    explicit VulkanBuffer(VulkanDevice& device, const CreateInfo& createInfo);
    ~VulkanBuffer();
    VulkanBuffer(const VulkanBuffer& other) = delete;
    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
    VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;
    void swap(VulkanBuffer& other) noexcept;

    void subDataLocal(const void* data, size_t offset, size_t size);
    void* mapMemory();
    void unmapMemory();
    // void subData(const void* data, size_t offset, size_t size);

    VkBuffer& getHandle() {
        return buffer;
    }

    const VkBuffer& getHandle() const {
        return buffer;
    }

    VmaAllocation getAllocation() const {
        return allocation;
    }

    void* getMappedPtr() const {
        return mappedPtr;
    }

    VkDeviceSize getSize() const {
        return bufferSize;
    }

    VkDescriptorType getDescriptorType() const {
        return descriptorType;
    }

    operator bool() const {
        return buffer != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{nullptr};
    VkBuffer buffer{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkDeviceSize bufferSize{0};
    void* mappedPtr{nullptr};
    VkDescriptorType descriptorType{};
};

struct ENGINE_API VulkanBufferBinding {
    uint32_t binding{0};
    const VulkanBuffer* uniform{nullptr};
};
} // namespace Engine
