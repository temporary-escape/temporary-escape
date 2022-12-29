#pragma once

#include "vg_device.hpp"

namespace Engine {
class VgRenderer : public VgDevice {
public:
    explicit VgRenderer(const Config& config);

    VgPipeline createPipeline(const VgPipeline::CreateInfo& createInfo);
    VgFramebuffer& getSwapChainFramebuffer();

    void beginRenderPass(const VgFramebuffer& framebuffer, const Vector2i& size);

    void beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo) {
        getCommandBuffer().beginRenderPass(renderPassInfo);
    }
    void endRenderPass() {
        getCommandBuffer().endRenderPass();
    }

    void setViewport(const Vector2i& pos, const Vector2i& size, const float minDepth = 0.0f,
                     const float maxDepth = 1.0f) {
        getCommandBuffer().setViewport(pos, size, minDepth, maxDepth);
    }

    void setScissor(const Vector2i& pos, const Vector2i& size) {
        getCommandBuffer().setScissor(pos, size);
    }

    void bindPipeline(const VgPipeline& pipeline) {
        getCommandBuffer().bindPipeline(pipeline);
        currentPipelineLayout = pipeline.getLayout();
    }

    void bindBuffers(const std::vector<VgVertexBufferBindRef>& buffers) {
        getCommandBuffer().bindBuffers(buffers);
    }

    void bindIndexBuffer(const VgBuffer& buffer, const VkDeviceSize offset, const VkIndexType indexType) {
        getCommandBuffer().bindIndexBuffer(buffer, offset, indexType);
    }

    void draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
              const uint32_t firstInstance) {
        getCommandBuffer().draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void drawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex,
                     const int32_t vertexOffset, const uint32_t firstInstance) {
        getCommandBuffer().drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void copyBuffer(const VgBuffer& src, const VgBuffer& dst, const VkBufferCopy& region) {
        getCommandBuffer().copyBuffer(src, dst, region);
    }

    void bindDescriptors(VgDescriptorSetLayout& layout, const std::vector<VgBufferBinding>& uniforms,
                         const std::vector<VgTextureBinding>& textures);

protected:
    void onFrameDraw(const Vector2i& viewport, float deltaTime) override;
    void onSwapChainChanged() override;

private:
    void createRenderPass();
    void createSwapChainFramebuffers();
    VgCommandBuffer& getCommandBuffer() {
        return commandBuffers.at(getCurrentFrameNum());
    }

    VgRenderPass renderPass;
    std::array<VgCommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers;
    std::vector<VgFramebuffer> swapChainFramebuffers;
    Vector2i lastViewportSize;
    VkPipelineLayout currentPipelineLayout{VK_NULL_HANDLE};
};
} // namespace Engine
