#pragma once

#include "../vulkan/vulkan_buffer.hpp"
#include "material.hpp"

namespace Engine {
struct Primitive {
    VulkanBuffer vbo;
    VulkanBuffer ibo;
    const Material* material{nullptr};
    uint32_t count{0};
    VkFormat indexType{VkFormat::VK_FORMAT_R8_UNORM};
};
} // namespace Engine
