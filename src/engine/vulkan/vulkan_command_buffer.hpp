#pragma once

#include "vulkan_buffer.hpp"
#include "vulkan_texture.hpp"
#include "vulkan_types.hpp"

namespace Engine {
struct ENGINE_API VulkanVertexBufferBindRef {
    const VulkanBuffer* buffer;
    VkDeviceSize offset;
};

class ENGINE_API VulkanRenderer;
class ENGINE_API VulkanCommandPool;
class ENGINE_API VulkanDescriptorSet;
class ENGINE_API VulkanFramebuffer;
class ENGINE_API VulkanPipeline;
class ENGINE_API VulkanDescriptorSetLayout;
class ENGINE_API VulkanRenderPass;
class ENGINE_API VulkanDescriptorPool;

struct ENGINE_API VulkanRenderPassBeginInfo {
    const VulkanRenderPass* renderPass{nullptr};
    const VulkanFramebuffer* framebuffer{nullptr};
    std::vector<VkClearValue> clearValues;
    Vector2i offset;
    Vector2i size;
};

class ENGINE_API VulkanCommandBuffer : public VulkanDisposable {
public:
    VulkanCommandBuffer() = default;
    explicit VulkanCommandBuffer(VulkanDevice& device, VulkanCommandPool& commandPool,
                                 VulkanDescriptorPool& descriptorPool);
    ~VulkanCommandBuffer();
    VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
    VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer& other) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer&& other) noexcept;
    void swap(VulkanCommandBuffer& other) noexcept;

    VkCommandBuffer& getHandle() {
        return commandBuffer;
    }

    const VkCommandBuffer& getHandle() const {
        return commandBuffer;
    }

    operator bool() const {
        return commandBuffer != VK_NULL_HANDLE;
    }

    void start(const VkCommandBufferBeginInfo& beginInfo);
    void end();

    void beginRenderPass(const VulkanRenderPassBeginInfo& renderPassInfo);
    void nextSubpass();
    void endRenderPass();

    void setViewport(const Vector2i& pos, const Vector2i& size, float minDepth = 0.0f, float maxDepth = 1.0f);
    void setScissor(const Vector2i& pos, const Vector2i& size);
    void bindPipeline(const VulkanPipeline& pipeline);
    void bindBuffers(const Span<VulkanVertexBufferBindRef>& buffers);
    void bindIndexBuffer(const VulkanBuffer& buffer, VkDeviceSize offset, VkIndexType indexType);
    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                     uint32_t firstInstance);
    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void copyImage(const VulkanTexture& src, VkImageLayout srcLayout, const VulkanTexture& dst, VkImageLayout dstLayout,
                   const VkImageCopy& region);
    void copyBuffer(const VulkanBuffer& src, const VulkanBuffer& dst, const VkBufferCopy& region);
    void copyBufferToImage(const VulkanBuffer& src, const VulkanTexture& dst, const VkBufferImageCopy& region);
    void copyBufferToImage(const VulkanBuffer& src, const VulkanTexture& dst, const Span<VkBufferImageCopy>& regions);
    void bindDescriptorSet(const VulkanDescriptorSet& descriptorSet, VkPipelineLayout pipelineLayout,
                           bool isCompute = false);
    void pipelineBarrier(const VkPipelineStageFlags& source, const VkPipelineStageFlags& destination,
                         VkImageMemoryBarrier& barrier);
    void bindDescriptors(const VulkanPipeline& pipeline, const VulkanDescriptorSetLayout& layout,
                         const Span<VulkanBufferBinding>& uniforms, const Span<VulkanTextureBinding>& textures,
                         const Span<VulkanTextureBinding>& inputs);
    void bindDescriptors(const VulkanPipeline& pipeline, const VulkanDescriptorSetLayout& layout,
                         VulkanDescriptorPool& pool, const Span<VulkanBufferBinding>& uniforms,
                         const Span<VulkanTextureBinding>& textures, const Span<VulkanTextureBinding>& inputs);
    void pushConstants(const VulkanPipeline& pipeline, const VkShaderStageFlags shaderStage, size_t offset, size_t size,
                       const void* data);
    void blitImage(const VulkanTexture& src, VkImageLayout srcLayout, const VulkanTexture& dst, VkImageLayout dstLayout,
                   const Span<VkImageBlit>& regions, VkFilter filter);
    void generateMipMaps(VulkanTexture& texture);
    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VulkanDescriptorPool* descriptorPool{nullptr};
};
} // namespace Engine
