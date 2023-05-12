#pragma once

#include "../vulkan/vulkan_buffer.hpp"

namespace Engine {
struct ENGINE_API Mesh : public VulkanDisposable {
    VulkanBuffer vbo;
    VulkanBuffer ibo;
    VkIndexType indexType{VK_INDEX_TYPE_UINT16};
    uint32_t count{0};
    uint32_t instances{1};

    operator bool() const {
        return vbo || ibo;
    }

    void destroy() override {
        vbo.destroy();
        ibo.destroy();
    }
};
} // namespace Engine
