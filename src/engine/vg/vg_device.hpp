#pragma once

#include "vg_buffer.hpp"
#include "vg_command_buffer.hpp"
#include "vg_command_pool.hpp"
#include "vg_descriptor_pool.hpp"
#include "vg_descriptor_set.hpp"
#include "vg_descriptor_set_layout.hpp"
#include "vg_framebuffer.hpp"
#include "vg_instance.hpp"
#include "vg_pipeline.hpp"
#include "vg_render_pass.hpp"
#include "vg_shader_module.hpp"
#include "vg_swap_chain.hpp"
#include "vg_sync_object.hpp"
#include "vg_texture.hpp"
#include "vg_uniform_buffer.hpp"

struct VmaAllocator_T;

namespace Engine {
class ENGINE_API VgDevice : public VgInstance {
public:
    explicit VgDevice(const Config& config);
    virtual ~VgDevice();

    VgShaderModule createShaderModule(const std::string& glsl, VkShaderStageFlagBits stage);
    VgShaderModule createShaderModule(const Path& path, VkShaderStageFlagBits stage);
    VgPipeline createPipeline(const VgRenderPass& renderPass, const VgPipeline::CreateInfo& createInfo);
    VgRenderPass createRenderPass(const VgRenderPass::CreateInfo& createInfo);
    VgFramebuffer createFramebuffer(const VgFramebuffer::CreateInfo& createInfo);
    VgSyncObject createSyncObject();
    VgCommandPool createCommandPool(const VgCommandPool::CreateInfo& createInfo);
    VgBuffer createBuffer(const VgBuffer::CreateInfo& createInfo);
    VgDescriptorSetLayout createDescriptorSetLayout(const VgDescriptorSetLayout::CreateInfo& createInfo);
    VgDescriptorPool createDescriptorPool();
    VgDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VgDescriptorSet createDescriptorSet(VgDescriptorPool& pool, VgDescriptorSetLayout& layout);
    VgUniformBuffer createUniformBuffer(size_t size, VgUniformBuffer::Usage usage);
    VgTexture createTexture(const VgTexture::CreateInfo& createInfo);

    VkDevice& getHandle() {
        return device;
    }

    const VkDevice& getHandle() const {
        return device;
    }

    void waitDeviceIdle();

    uint32_t getSwapChainFramebufferIndex() const {
        return swapChainFramebufferIndex;
    }
    void submitCommandBuffer(const VgCommandBuffer& commandBuffer);
    void submitPresentQueue();
    void recreateSwapChain();

    VgSwapChain& getSwapChain() {
        return swapChain;
    }

    VgCommandPool& getCommandPool() {
        return commandPool;
    }

    VmaAllocator_T* getAllocator() const {
        return allocator;
    }

    uint32_t getCurrentFrameNum() const {
        return currentFrameNum;
    }

    void uploadBufferData(const void* data, size_t size, VgBuffer& dst);
    void copyBufferToImage(const VgBuffer& buffer, VgTexture& texture, int level, int layer, const VkOffset3D& offset,
                           const VkExtent3D& extent);

    void dispose(std::shared_ptr<VgDisposable> disposable);

    void getGpuMemoryStats();

    void transitionImageLayout(VgTexture& texture, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

protected:
    virtual void onSwapChainChanged() = 0;
    void onNextFrame() override;
    void onExit() override;

    VgSyncObject& getCurrentSyncObject() {
        return syncObjects[currentFrameNum];
    }

    VgDescriptorPool& getCurrentDescriptorPool() {
        return descriptorPools[currentFrameNum];
    }

private:
    void cleanup();
    void destroyDisposables();

    const Config& config;
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    VgCommandPool commandPool;
    VgSwapChain swapChain;
    VgSyncObject syncObjects[MAX_FRAMES_IN_FLIGHT];
    VgDescriptorPool descriptorPools[MAX_FRAMES_IN_FLIGHT];
    VgBuffer transferBuffer;
    VmaAllocator_T* allocator{VK_NULL_HANDLE};
    uint32_t swapChainFramebufferIndex{0};
    VkDeviceSize alignedFlushSize{0};
    size_t currentFrameNum{0};
    std::list<std::shared_ptr<VgDisposable>> disposables;
    bool exitTriggered{false};
};
} // namespace Engine
