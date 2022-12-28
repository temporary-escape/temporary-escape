#pragma once

#include "vg_buffer.hpp"
#include "vg_command_buffer.hpp"
#include "vg_framebuffer.hpp"
#include "vg_instance.hpp"
#include "vg_pipeline.hpp"
#include "vg_render_pass.hpp"
#include "vg_shader_module.hpp"
#include "vg_swap_chain.hpp"
#include "vg_sync_object.hpp"

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
    VgCommandBuffer createCommandBuffer();
    VgBuffer createBuffer(const VgBuffer::CreateInfo& createInfo);

    VkDevice getHandle() const {
        return device;
    }

    void waitDeviceIdle();

    uint32_t getSwapChainFramebufferIndex() const {
        return swapChainFramebufferIndex;
    }
    void submitCommandBuffer(const VgCommandBuffer& commandBuffer);
    void submitPresentQueue();
    void recreateSwapChain();

    const VgSwapChain& getSwapChain() const {
        return swapChain;
    }

    VmaAllocator_T* getAllocator() const {
        return allocator;
    }

    uint32_t getCurrentFrameNum() const {
        return currentFrameNum;
    }

    void uploadBufferData(const void* data, size_t size, VgBuffer& dst);

    void dispose(std::shared_ptr<VgDisposable> disposable);

protected:
    virtual void onSwapChainChanged() = 0;
    void onNextFrame() override;
    void onExit() override;

    VgSyncObject& getCurrentSyncObject() {
        return syncObjects[currentFrameNum];
    }

private:
    void cleanup();
    void destroyDisposables();

    const Config& config;
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VgSwapChain swapChain;
    VgSyncObject syncObjects[MAX_FRAMES_IN_FLIGHT];
    VgCommandBuffer transferCommandBuffer;
    VgBuffer transferBuffer;
    VmaAllocator_T* allocator{VK_NULL_HANDLE};
    uint32_t swapChainFramebufferIndex{0};
    VkDeviceSize alignedFlushSize{0};
    size_t currentFrameNum{0};
    std::list<std::shared_ptr<VgDisposable>> disposables;
    bool exitTriggered{false};
};
} // namespace Engine
