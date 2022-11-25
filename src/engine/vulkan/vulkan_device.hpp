#pragma once

#include "../config.hpp"
#include "../math/matrix.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_framebuffer.hpp"
#include "vulkan_pipeline.hpp"
#include "vulkan_texture.hpp"
#include "vulkan_vertex_input_format.hpp"

struct GLFWwindow;

namespace Engine {
using VulkanBlendState = VezColorBlendAttachmentState;

struct BlitImageInfo {
    struct Resource {
        Vector3i offset{0, 0, 0};
        Vector3i size{0, 0, 0};
        int mipLevel{0};
        int baseArrayLayer{0};
        int layerCount{0};
    };

    Resource src;
    Resource dst;
};

class ENGINE_API VulkanDevice {
public:
    explicit VulkanDevice(const Config& config);
    virtual ~VulkanDevice() = default;

    virtual Vector2i getWindowSize() = 0;

    void setViewport(const Vector2i& pos, const Vector2i& size);
    void setScissor(const Vector2i& pos, const Vector2i& size);
    void setViewportState();

    [[nodiscard]] VulkanTexture::Format getSwapchainFormat() const;
    VulkanBuffer createBuffer(VulkanBuffer::Type type, VulkanBuffer::Usage usage, size_t size);
    VulkanPipeline createPipeline(const std::vector<ShaderSource>& sources);
    VulkanTexture createTexture(const VulkanTexture::Descriptor& desc);
    VulkanVertexInputFormat createVertexInputFormat(const std::vector<VulkanVertexInputFormat::Binding>& bindings);
    VulkanFramebuffer createFramebuffer(const Vector2i& size,
                                        const std::vector<VulkanFramebufferAttachment>& attachments);

    void bindVertexBuffer(const VulkanBuffer& buffer, size_t offset);
    void bindIndexBuffer(const VulkanBuffer& buffer, size_t offset, VkIndexType indexType);
    void bindUniformBuffer(const VulkanBuffer& buffer, uint32_t binding, size_t offset = 0);
    void bindPipeline(const VulkanPipeline& pipeline);
    void bindVertexInputFormat(const VulkanVertexInputFormat& vertexInputFormat);
    void bindTexture(const VulkanTexture& texture, uint32_t binding);

    void beginRenderPass(VulkanFramebuffer& fbo, const std::vector<VulkanFramebufferAttachmentReference>& attachments);
    void nextSubpass();
    void endRenderPass();
    void setRasterization(VkPolygonMode mode = VkPolygonMode::VK_POLYGON_MODE_FILL,
                          VkCullModeFlags cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT,
                          VkFrontFace frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    void setInputAssembly(VkPrimitiveTopology topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                          bool restart = false);
    void setDepthStencilState(bool write, bool test, VkCompareOp compareOp = VK_COMPARE_OP_LESS_OR_EQUAL);
    void setBlendState(const std::vector<VulkanBlendState>& blendStates);

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                     uint32_t firstInstance);
    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

    void pushConstant(uint32_t offset, bool value);
    void pushConstant(uint32_t offset, float value);
    void pushConstant(uint32_t offset, const Vector2& value);
    void pushConstant(uint32_t offset, const Matrix4& value);
    void pushConstant(uint32_t offset, const Color4& value);
    void startCommandBuffer();
    void endCommandBuffer();
    void submitQueue();
    void submitPresentQueue(VulkanTexture& front);
    void deviceWaitIdle();

    void blitImage(VulkanTexture& source, VulkanTexture& target, const BlitImageInfo& blitInfo, VkFilter filter);

protected:
    bool getValidationLayerSupported();

    void initInstance(const std::string& name, std::vector<const char*> instanceLayers,
                      std::vector<const char*> instanceExtensions);
    void initSurface(GLFWwindow* window);
    void reset();
    void resizeDefaultFramebuffer(const Vector2i& size);

private:
    VkShaderModule CreateShaderModule(const std::string& code, const std::string& entryPoint,
                                      VkShaderStageFlagBits stage);

    const Config& config;
    std::vector<std::string> deviceExtensions;
    VkInstance instance = VK_NULL_HANDLE;
    int physicalDeviceIndex = 0;
    VkSampleCountFlagBits sampleCountFlag = VK_SAMPLE_COUNT_1_BIT;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VezSwapchain swapchain = VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
};
} // namespace Engine
