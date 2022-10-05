#pragma once

#include "../Vulkan/VulkanBuffer.hpp"
#include "Material.hpp"

namespace Engine {
struct Primitive {
    VulkanBuffer vbo;
    VulkanBuffer ibo;
    const Material* material{nullptr};
    uint32_t count{0};
    VkFormat indexType{VkFormat::VK_FORMAT_R8_UNORM};
    VkPrimitiveTopology topology{VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    VulkanVertexInputFormat vboFormat;
};
} // namespace Engine
