#pragma once

#include "../utils/path.hpp"
#include "vg_buffer.hpp"
#include "vg_pipeline.hpp"
#include "vg_types.hpp"

namespace Engine {
struct VgVertexBufferBindRef {
    std::reference_wrapper<VgBuffer> buffer;
    VkDeviceSize offset;
};

class VgCommandBuffer {
public:
    VgCommandBuffer() = default;
    explicit VgCommandBuffer(const Config& config, VkDevice device, VkCommandPool commandPool);
    ~VgCommandBuffer();
    VgCommandBuffer(const VgCommandBuffer& other) = delete;
    VgCommandBuffer(VgCommandBuffer&& other) noexcept;
    VgCommandBuffer& operator=(const VgCommandBuffer& other) = delete;
    VgCommandBuffer& operator=(VgCommandBuffer&& other) noexcept;
    void swap(VgCommandBuffer& other) noexcept;

    VkCommandBuffer getHandle() const {
        return commandBuffer;
    }

    operator bool() const {
        return commandBuffer != VK_NULL_HANDLE;
    }

    void startCommandBuffer(const VkCommandBufferBeginInfo& beginInfo);
    void endCommandBuffer();

    void beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo);
    void endRenderPass();

    void setViewport(const Vector2i& pos, const Vector2i& size, float minDepth = 0.0f, float maxDepth = 1.0f);
    void setScissor(const Vector2i& pos, const Vector2i& size);
    void bindPipeline(const VgPipeline& pipeline);
    void bindBuffers(const std::vector<VgVertexBufferBindRef>& buffers);
    void drawVertices(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void copyBuffer(const VgBuffer& src, const VgBuffer& dst, const VkBufferCopy& region);
    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
};
} // namespace Engine
