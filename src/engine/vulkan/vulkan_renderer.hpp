#pragma once

#include "vulkan_allocator.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_command_buffer.hpp"
#include "vulkan_command_pool.hpp"
#include "vulkan_descriptor_pool.hpp"
#include "vulkan_descriptor_set.hpp"
#include "vulkan_descriptor_set_layout.hpp"
#include "vulkan_device.hpp"
#include "vulkan_double_buffer.hpp"
#include "vulkan_fence.hpp"
#include "vulkan_framebuffer.hpp"
#include "vulkan_pipeline.hpp"
#include "vulkan_query_pool.hpp"
#include "vulkan_render_pass.hpp"
#include "vulkan_semaphore.hpp"
#include "vulkan_shader.hpp"
#include "vulkan_swap_chain.hpp"
#include "vulkan_texture.hpp"

struct ktxVulkanDeviceInfo;

namespace Engine {
using VulkanSemaphoreOpt = std::optional<std::reference_wrapper<VulkanSemaphore>>;
using VulkanFenceOpt = std::optional<std::reference_wrapper<VulkanFence>>;

class ENGINE_API VulkanRenderer : public VulkanDevice {
public:
    explicit VulkanRenderer(const Config& config);
    virtual ~VulkanRenderer();

    VulkanShader createShaderModule(const std::string& glsl, VkShaderStageFlagBits stage);
    VulkanShader createShaderModule(const Path& path, VkShaderStageFlagBits stage);
    VulkanPipeline createPipeline(const VulkanRenderPass& renderPass, const VulkanPipeline::CreateInfo& createInfo);
    VulkanPipeline createPipeline(const VulkanPipeline::CreateComputeInfo& createInfo);
    VulkanPipeline createPipeline(const VulkanPipeline::CreateInfo& createInfo) {
        return createPipeline(renderPass, createInfo);
    }
    VulkanRenderPass createRenderPass(const VulkanRenderPass::CreateInfo& createInfo);
    VulkanFramebuffer createFramebuffer(const VulkanFramebuffer::CreateInfo& createInfo);
    VulkanFence createFence();
    VulkanSemaphore createSemaphore(const VulkanSemaphore::CreateInfo& createInfo);
    VulkanCommandPool createCommandPool(const VulkanCommandPool::CreateInfo& createInfo);
    VulkanCommandBuffer createCommandBuffer();
    VulkanCommandBuffer createComputeCommandBuffer();
    VulkanBuffer createBuffer(const VulkanBuffer::CreateInfo& createInfo);
    VulkanDescriptorSetLayout createDescriptorSetLayout(const VulkanDescriptorSetLayout::CreateInfo& createInfo);
    VulkanDescriptorPool createDescriptorPool();
    VulkanDescriptorPool createDescriptorPool(const VulkanDescriptorPool::CreateInfo& createInfo);
    VulkanDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VulkanDoubleBuffer createDoubleBuffer(const VulkanBuffer::CreateInfo& createInfo);
    VulkanTexture createTexture(const VulkanTexture::CreateInfo& createInfo);
    VulkanQueryPool createQueryPool(const VulkanQueryPool::CreateInfo& createInfo);

    void waitDeviceIdle();
    void waitQueueIdle();
    void submitCommandBuffer(const VulkanCommandBuffer& commandBuffer);
    void submitCommandBuffer(const VulkanCommandBuffer& commandBuffer, VkPipelineStageFlags waitStages,
                             const VulkanSemaphoreOpt& wait, const VulkanSemaphoreOpt& signal,
                             const VulkanFenceOpt& fence);
    void submitPresentCommandBuffer(const VulkanCommandBuffer& commandBuffer, VulkanSemaphore* wait = nullptr);
    void submitPresentQueue();
    void recreateSwapChain();
    void copyDataToBuffer(VulkanBuffer& buffer, const void* data, size_t size);
    void copyDataToImage(VulkanTexture& texture, int level, const Vector2i& offset, int layer, const Vector2i& size,
                         const void* data, const std::optional<size_t>& dataSize = std::nullopt);
    void copyImageToImage(VulkanTexture& texture, int level, const Vector2i& offset, int layer, const Vector2i& size,
                          const VulkanTexture& source);
    void copyBufferToImage(const VulkanBuffer& buffer, VulkanTexture& texture, int level, int layer,
                           const VkOffset3D& offset, const VkExtent3D& extent);
    // void getGpuMemoryStats();
    void transitionImageLayout(VulkanTexture& texture, VkImageLayout oldLayout, VkImageLayout newLayout);
    void generateMipMaps(VulkanTexture& texture);

    template <typename T> void dispose(T&& resource) {
        if (!resource) {
            return;
        }
        disposables.at(getCurrentFrameNum()).push_back(std::make_shared<T>(std::forward<T>(resource)));
    }

    uint32_t getSwapChainFramebufferIndex() const {
        return swapChainFramebufferIndex;
    }

    VulkanSwapChain& getSwapChain() {
        return swapChain;
    }

    const VulkanSwapChain& getSwapChain() const {
        return swapChain;
    }

    VulkanCommandPool& getCommandPool() {
        return commandPool;
    }

    const VulkanCommandPool& getCommandPool() const {
        return commandPool;
    }

    uint32_t getCurrentFrameNum() const {
        return static_cast<uint32_t>(currentFrameNum);
    }

    VulkanFence& getCurrentInFlightFence() {
        return inFlightFence.at(currentFrameNum);
    }

    const VulkanFence& getCurrentInFlightFence() const {
        return inFlightFence.at(currentFrameNum);
    }

    VulkanSemaphore& getCurrentRenderFinishedSemaphore() {
        return renderFinishedSemaphore.at(currentFrameNum);
    }

    const VulkanSemaphore& getCurrentRenderFinishedSemaphore() const {
        return renderFinishedSemaphore.at(currentFrameNum);
    }

    VulkanSemaphore& getCurrentImageAvailableSemaphore() {
        return imageAvailableSemaphore.at(currentFrameNum);
    }

    const VulkanSemaphore& getCurrentImageAvailableSemaphore() const {
        return imageAvailableSemaphore.at(currentFrameNum);
    }

    VulkanDescriptorPool& getCurrentDescriptorPool() {
        return descriptorPools.at(currentFrameNum);
    }

    const VulkanDescriptorPool& getCurrentDescriptorPool() const {
        return descriptorPools.at(currentFrameNum);
    }

    VulkanFramebuffer& getSwapChainFramebuffer() {
        return swapChainFramebuffers.at(getSwapChainFramebufferIndex());
    }

    const VulkanFramebuffer& getSwapChainFramebuffer() const {
        return swapChainFramebuffers.at(getSwapChainFramebufferIndex());
    }

    VulkanRenderPass& getRenderPass() {
        return renderPass;
    }

    const VulkanRenderPass& getRenderPass() const {
        return renderPass;
    }

    bool canBeMipMapped(const VkFormat format) const;

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
    VulkanCommandPool commandPool;
    VulkanCommandPool commandComputePool;
    VulkanSwapChain swapChain;
    std::array<VulkanSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphore;
    std::array<VulkanSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphore;
    std::array<VulkanFence, MAX_FRAMES_IN_FLIGHT> inFlightFence;
    std::array<VulkanDescriptorPool, MAX_FRAMES_IN_FLIGHT> descriptorPools;
    VulkanBuffer transferBuffer;
    uint32_t swapChainFramebufferIndex{0};
    VkDeviceSize alignedFlushSize{0};
    size_t currentFrameNum{0};
    std::array<std::list<std::shared_ptr<VulkanDisposable>>, MAX_FRAMES_IN_FLIGHT> disposables;
    bool exitTriggered{false};
    VulkanRenderPass renderPass;
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    Vector2i lastViewportSize;
};
} // namespace Engine
