#pragma once

#include "vg_allocator.hpp"
#include "vg_buffer.hpp"
#include "vg_command_buffer.hpp"
#include "vg_command_pool.hpp"
#include "vg_descriptor_pool.hpp"
#include "vg_descriptor_set.hpp"
#include "vg_descriptor_set_layout.hpp"
#include "vg_device.hpp"
#include "vg_double_buffer.hpp"
#include "vg_framebuffer.hpp"
#include "vg_pipeline.hpp"
#include "vg_render_pass.hpp"
#include "vg_shader_module.hpp"
#include "vg_swap_chain.hpp"
#include "vg_sync_object.hpp"
#include "vg_texture.hpp"

struct VmaAllocator_T;

namespace Engine {
class ENGINE_API VgRenderer : public VgDevice {
public:
    explicit VgRenderer(const Config& config);
    virtual ~VgRenderer();

    VgShaderModule createShaderModule(const std::string& glsl, VkShaderStageFlagBits stage);
    VgShaderModule createShaderModule(const Path& path, VkShaderStageFlagBits stage);
    VgPipeline createPipeline(const VgRenderPass& renderPass, const VgPipeline::CreateInfo& createInfo);
    VgPipeline createPipeline(const VgPipeline::CreateInfo& createInfo) {
        return createPipeline(renderPass, createInfo);
    }
    VgRenderPass createRenderPass(const VgRenderPass::CreateInfo& createInfo);
    VgFramebuffer createFramebuffer(const VgFramebuffer::CreateInfo& createInfo);
    VgSyncObject createSyncObject();
    VgCommandPool createCommandPool(const VgCommandPool::CreateInfo& createInfo);
    VgCommandBuffer createCommandBuffer();
    VgBuffer createBuffer(const VgBuffer::CreateInfo& createInfo);
    VgDescriptorSetLayout createDescriptorSetLayout(const VgDescriptorSetLayout::CreateInfo& createInfo);
    VgDescriptorPool createDescriptorPool();
    VgDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VgDescriptorSet createDescriptorSet(VgDescriptorPool& pool, VgDescriptorSetLayout& layout);
    VgDoubleBuffer createDoubleBuffer(const VgBuffer::CreateInfo& createInfo);
    VgTexture createTexture(const VgTexture::CreateInfo& createInfo);
    void waitDeviceIdle();
    void submitCommandBuffer(const VgCommandBuffer& commandBuffer);
    void submitPresentQueue();
    void recreateSwapChain();
    void uploadBufferData(const void* data, size_t size, VgBuffer& dst);
    void copyBufferToImage(const VgBuffer& buffer, VgTexture& texture, int level, int layer, const VkOffset3D& offset,
                           const VkExtent3D& extent);
    void dispose(std::shared_ptr<VgDisposable> disposable);
    void getGpuMemoryStats();
    void transitionImageLayout(VgTexture& texture, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    uint32_t getSwapChainFramebufferIndex() const {
        return swapChainFramebufferIndex;
    }

    VgSwapChain& getSwapChain() {
        return swapChain;
    }

    const VgSwapChain& getSwapChain() const {
        return swapChain;
    }

    VgCommandPool& getCommandPool() {
        return commandPool;
    }

    const VgCommandPool& getCommandPool() const {
        return commandPool;
    }

    VgAllocator& getAllocator() {
        return allocator;
    }

    const VgAllocator& getAllocator() const {
        return allocator;
    }

    uint32_t getCurrentFrameNum() const {
        return currentFrameNum;
    }

    VgSyncObject& getCurrentSyncObject() {
        return syncObjects.at(currentFrameNum);
    }

    const VgSyncObject& getCurrentSyncObject() const {
        return syncObjects.at(currentFrameNum);
    }

    VgDescriptorPool& getCurrentDescriptorPool() {
        return descriptorPools.at(currentFrameNum);
    }

    const VgDescriptorPool& getCurrentDescriptorPool() const {
        return descriptorPools.at(currentFrameNum);
    }

    VgFramebuffer& getSwapChainFramebuffer() {
        return swapChainFramebuffers.at(getSwapChainFramebufferIndex());
    }

    const VgFramebuffer& getSwapChainFramebuffer() const {
        return swapChainFramebuffers.at(getSwapChainFramebufferIndex());
    }

    VgRenderPass& getRenderPass() {
        return renderPass;
    }

    const VgRenderPass& getRenderPass() const {
        return renderPass;
    }

private:
    void onNextFrame() override;
    void onExit() override;
    void onFrameDraw(const Vector2i& viewport, float deltaTime) override;
    void createRenderPass();
    void createSwapChainFramebuffers();
    void destroy();
    void destroyDisposables();
    void destroyDisposablesAll();

    const Config& config;
    VgCommandPool commandPool;
    VgSwapChain swapChain;
    std::array<VgSyncObject, MAX_FRAMES_IN_FLIGHT> syncObjects;
    std::array<VgDescriptorPool, MAX_FRAMES_IN_FLIGHT> descriptorPools;
    VgBuffer transferBuffer;
    VgAllocator allocator;
    uint32_t swapChainFramebufferIndex{0};
    VkDeviceSize alignedFlushSize{0};
    size_t currentFrameNum{0};
    std::array<std::list<std::shared_ptr<VgDisposable>>, MAX_FRAMES_IN_FLIGHT> disposables;
    bool exitTriggered{false};
    VgRenderPass renderPass;
    std::vector<VgFramebuffer> swapChainFramebuffers;
    Vector2i lastViewportSize;
};
} // namespace Engine
