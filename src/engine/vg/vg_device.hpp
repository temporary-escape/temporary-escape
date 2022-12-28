#pragma once

#include "vg_command_buffer.hpp"
#include "vg_framebuffer.hpp"
#include "vg_instance.hpp"
#include "vg_pipeline.hpp"
#include "vg_render_pass.hpp"
#include "vg_shader_module.hpp"
#include "vg_swap_chain.hpp"
#include "vg_sync_object.hpp"

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

protected:
    virtual void onSwapChainChanged() = 0;
    void onNextFrame() override;
    void onExit() override;

private:
    void cleanup();

    const Config& config;
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VgSwapChain swapChain;
    VgSyncObject syncObject;
    uint32_t swapChainFramebufferIndex{0};
};
} // namespace Engine
