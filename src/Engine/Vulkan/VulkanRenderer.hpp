#pragma once

#include "VulkanAllocator.hpp"
#include "VulkanArrayBuffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanDescriptorPool.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanDevice.hpp"
#include "VulkanDoubleBuffer.hpp"
#include "VulkanFence.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanImageView.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanQueryPool.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanSemaphore.hpp"
#include "VulkanShader.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanTexture.hpp"

struct ktxVulkanDeviceInfo;

namespace Engine {
using VulkanSemaphoreOpt = std::optional<std::reference_wrapper<const VulkanSemaphore>>;
using VulkanFenceOpt = std::optional<std::reference_wrapper<const VulkanFence>>;
struct VulkanSubmitInfo {
    VkPipelineStageFlags waitMask{0};
    const VulkanSemaphore* signal{nullptr};
    const VulkanSemaphore* wait{nullptr};
    const VulkanFence* fence{nullptr};
    bool compute{false};
    bool present{false};
};

class ENGINE_API VulkanRenderer : public VulkanDevice {
public:
    explicit VulkanRenderer(const Config& config);
    virtual ~VulkanRenderer();

    VulkanShader createShaderModule(const Path& path, VkShaderStageFlagBits stage);
    VulkanShader createShaderModule(const Span<uint8_t>& spirv, VkShaderStageFlagBits stage);
    VulkanPipeline createPipeline(const VulkanRenderPass& renderPass, const VulkanPipeline::CreateInfo& createInfo);
    VulkanPipeline createPipeline(const VulkanPipeline::CreateComputeInfo& createInfo);
    VulkanPipeline createPipeline(const VulkanPipeline::CreateInfo& createInfo) {
        return createPipeline(renderPass, createInfo);
    }
    VulkanRenderPass createRenderPass(const VulkanRenderPass::CreateInfo& createInfo);
    VulkanFramebuffer createFramebuffer(const VulkanFramebuffer::CreateInfo& createInfo);
    VulkanFence createFence();
    VulkanSemaphore createSemaphore();
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
    VulkanImageView createImageView(const VulkanImageView::CreateInfo& createInfo);
    VulkanQueryPool createQueryPool(const VulkanQueryPool::CreateInfo& createInfo);

    void waitDeviceIdle();
    void waitQueueIdle();
    void submitCommandBuffer(const VulkanCommandBuffer& commandBuffer, const VulkanSubmitInfo& info);
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

    float getRenderTime() const {
        return renderTime;
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
    bool framebufferResized{false};
    float renderTime{0.0f};
};
} // namespace Engine
