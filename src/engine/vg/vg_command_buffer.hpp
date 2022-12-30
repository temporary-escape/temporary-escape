#pragma once

#include "vg_buffer.hpp"
#include "vg_texture.hpp"
#include "vg_types.hpp"

namespace Engine {
struct VgVertexBufferBindRef {
    std::reference_wrapper<VgBuffer> buffer;
    VkDeviceSize offset;
};

class VgRenderer;
class VgCommandPool;
class VgDescriptorSet;
class VgFramebuffer;
class VgPipeline;
class VgDescriptorSetLayout;
class VgRenderPass;

struct VgRenderPassBeginInfo {
    VgRenderPass* renderPass{nullptr};
    VgFramebuffer* framebuffer{nullptr};
    std::vector<VkClearValue> clearValues;
    Vector2i offset;
    Vector2i size;
};

class VgCommandBuffer {
public:
    VgCommandBuffer() = default;
    explicit VgCommandBuffer(const Config& config, VgRenderer& device, VgCommandPool& commandPool);
    ~VgCommandBuffer();
    VgCommandBuffer(const VgCommandBuffer& other) = delete;
    VgCommandBuffer(VgCommandBuffer&& other) noexcept;
    VgCommandBuffer& operator=(const VgCommandBuffer& other) = delete;
    VgCommandBuffer& operator=(VgCommandBuffer&& other) noexcept;
    void swap(VgCommandBuffer& other) noexcept;

    VkCommandBuffer& getHandle() {
        return state->commandBuffer;
    }

    const VkCommandBuffer& getHandle() const {
        return state->commandBuffer;
    }

    operator bool() const {
        return state && state->commandBuffer != VK_NULL_HANDLE;
    }

    void start(const VkCommandBufferBeginInfo& beginInfo);
    void end();

    void beginRenderPass(const VgRenderPassBeginInfo& renderPassInfo);
    void endRenderPass();

    void setViewport(const Vector2i& pos, const Vector2i& size, float minDepth = 0.0f, float maxDepth = 1.0f);
    void setScissor(const Vector2i& pos, const Vector2i& size);
    void bindPipeline(const VgPipeline& pipeline);
    void bindBuffers(const std::vector<VgVertexBufferBindRef>& buffers);
    void bindIndexBuffer(const VgBuffer& buffer, VkDeviceSize offset, VkIndexType indexType);
    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                     uint32_t firstInstance);
    void copyBuffer(const VgBuffer& src, const VgBuffer& dst, const VkBufferCopy& region);
    void copyBufferToImage(const VgBuffer& src, const VgTexture& dst, const VkBufferImageCopy& region);
    void bindDescriptorSet(const VgDescriptorSet& descriptorSet, VkPipelineLayout pipelineLayout);
    void pipelineBarrier(const VkPipelineStageFlags& source, const VkPipelineStageFlags& destination,
                         VkImageMemoryBarrier& barrier);
    void bindDescriptors(VgPipeline& pipeline, VgDescriptorSetLayout& layout,
                         const std::vector<VgBufferBinding>& uniforms, const std::vector<VgTextureBinding>& textures);
    void destroy();
    void free();

private:
    class CommandBufferState : public VgDisposable {
    public:
        void destroy() override;

        VgRenderer* device{nullptr};
        VkCommandPool commandPool{VK_NULL_HANDLE};
        VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    };
    std::shared_ptr<CommandBufferState> state;
};
} // namespace Engine
