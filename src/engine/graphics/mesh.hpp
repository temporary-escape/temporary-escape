#pragma once

#include "../vulkan/vulkan_buffer.hpp"

namespace Engine {
struct ENGINE_API Mesh {
    VulkanBuffer vbo;
    VulkanBuffer ibo;
    VkIndexType indexType{VK_INDEX_TYPE_UINT16};
    uint32_t count{0};
    uint32_t instances{1};
};
} // namespace Engine
