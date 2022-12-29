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
        getCurrentCommandBuffer().beginRenderPass(renderPassInfo);
    }
    void endRenderPass() {
        getCurrentCommandBuffer().endRenderPass();
    }

    void setViewport(const Vector2i& pos, const Vector2i& size, const float minDepth = 0.0f,
                     const float maxDepth = 1.0f) {
        getCurrentCommandBuffer().setViewport(pos, size, minDepth, maxDepth);
    }

    void setScissor(const Vector2i& pos, const Vector2i& size) {
        getCurrentCommandBuffer().setScissor(pos, size);
    }

    void bindPipeline(const VgPipeline& pipeline) {
        getCurrentCommandBuffer().bindPipeline(pipeline);
        currentPipelineLayout = pipeline.getLayout();
    }

    void bindBuffers(const std::vector<VgVertexBufferBindRef>& buffers) {
        getCurrentCommandBuffer().bindBuffers(buffers);
    }

    void drawVertices(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
                      const uint32_t firstInstance) {
        getCurrentCommandBuffer().drawVertices(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void copyBuffer(const VgBuffer& src, const VgBuffer& dst, const VkBufferCopy& region) {
        getCurrentCommandBuffer().copyBuffer(src, dst, region);
    }

    void bindDescriptors(VgDescriptorSetLayout& layout, const std::vector<VgUniformBufferBinding>& uniforms);

protected:
    void render(const Vector2i& viewport, float deltaTime) override;
    void onSwapChainChanged() override;
    virtual void draw(const Vector2i& viewport, float deltaTime) = 0;

private:
    void createRenderPass();
    void createSwapChainFramebuffers();
    VgCommandBuffer& getCurrentCommandBuffer() {
        return commandBuffers[getCurrentFrameNum()];
    }

    VgRenderPass renderPass;
    VgCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    std::vector<VgFramebuffer> swapChainFramebuffers;
    Vector2i lastViewportSize;
    VkPipelineLayout currentPipelineLayout{VK_NULL_HANDLE};
};
} // namespace Engine
