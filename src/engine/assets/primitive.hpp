#pragma once

#include "../vulkan/vulkan_buffer.hpp"
#include "material.hpp"

namespace Engine {
struct Primitive {
    Primitive() = default;
    Primitive(const Primitive& other) = delete;
    Primitive(Primitive&& other) = default;
    Primitive& operator=(const Primitive& other) = delete;
    Primitive& operator=(Primitive&& other) = default;

    VulkanBuffer vbo;
    VulkanBuffer ibo;
    const Material* material{nullptr};
    uint32_t count{0};
    VkIndexType indexType{VkIndexType::VK_INDEX_TYPE_UINT32};
};
} // namespace Engine
