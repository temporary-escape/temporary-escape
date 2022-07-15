#pragma once

#include "../Math/Matrix.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanPipeline.hpp"

struct GLFWwindow;

namespace Engine {
class VulkanDevice {
public:
    VulkanDevice() = default;

    void init(GLFWwindow* window);
    void reset();

    VulkanBuffer createBuffer(VulkanBuffer::Type type, VulkanBuffer::Usage usage, size_t size);
    VulkanPipeline createPipeline(const std::vector<ShaderSource>& sources);

    void bindVertexBuffer(VulkanBuffer& buffer, size_t offset = 0);
    void bindIndexBuffer(VulkanBuffer& buffer, size_t offset = 0);
    void bindUniformBuffer(VulkanBuffer& buffer, size_t offset = 0);
    void bindPipeline(VulkanPipeline& pipeline);

    void startCommandBuffer();
    void endCommandBuffer();
    void submitQueue();

private:
    VkShaderModule CreateShaderModule(const std::string& code, const std::string& entryPoint,
                                      VkShaderStageFlagBits stage);

    std::string m_name;
    int m_width = 0, m_height = 0;
    int m_physicalDeviceIndex = 0;
    bool m_enableValidationLayers = false;
    bool m_manageFramebuffer = false;
    VkSampleCountFlagBits m_sampleCountFlag = VK_SAMPLE_COUNT_1_BIT;
    std::vector<std::string> m_deviceExtensions;
    GLFWwindow* m_window = nullptr;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VezSwapchain m_swapchain = VK_NULL_HANDLE;

    struct {
        VkImage colorImage = VK_NULL_HANDLE;
        VkImageView colorImageView = VK_NULL_HANDLE;
        VkImage depthStencilImage = VK_NULL_HANDLE;
        VkImageView depthStencilImageView = VK_NULL_HANDLE;
        VezFramebuffer handle = VK_NULL_HANDLE;
    } m_framebuffer;

    bool m_quitSignaled = false;
    std::string m_windowTitleText;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
};
} // namespace Engine
