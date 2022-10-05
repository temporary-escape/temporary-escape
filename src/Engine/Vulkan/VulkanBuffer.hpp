#pragma once

#include "../Library.hpp"
#include "VEZ/VEZ.h"
#include <type_traits>

namespace Engine {
class VulkanBuffer {
public:
    enum class Usage {
        Static = VEZ_MEMORY_GPU_ONLY,
        Dynamic = VEZ_MEMORY_CPU_TO_GPU,
    };

    enum class Type {
        Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        Uniform = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    };

    NON_COPYABLE(VulkanBuffer);

    VulkanBuffer() = default;
    explicit VulkanBuffer(VkDevice device, Type type, Usage usage, size_t size);
    ~VulkanBuffer();
    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;
    void swap(VulkanBuffer& other) noexcept;

    void subData(const void* data, size_t offset, size_t size);
    void reset();

    void* mapPtr(size_t size);
    template <typename T> T* map() {
        return reinterpret_cast<T*>(mapPtr(sizeof(T)));
    }
    void unmap();

    [[nodiscard]] size_t getSize() const {
        return size;
    }

    [[nodiscard]] VkBuffer& getHandle() {
        return buffer;
    }

    [[nodiscard]] const VkBuffer& getHandle() const {
        return buffer;
    }

    [[nodiscard]] operator bool() const {
        return buffer != VK_NULL_HANDLE;
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};
    size_t size{0};
};

static_assert(std::is_move_constructible<VulkanBuffer>::value, "VulkanBuffer must be move constructible");
static_assert(std::is_move_assignable<VulkanBuffer>::value, "VulkanBuffer must be move assignable");
} // namespace Engine
