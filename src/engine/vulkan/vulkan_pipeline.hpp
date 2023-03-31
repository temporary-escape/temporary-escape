#pragma once

#include "../utils/path.hpp"
#include "vulkan_render_pass.hpp"
#include "vulkan_shader.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanPipeline : public VulkanDisposable {
public:
    struct CreateComputeInfo {
        VulkanShader* shaderModule{nullptr};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    };

    struct CreateInfo {
        std::vector<VulkanShader*> shaderModules;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        uint32_t subpass{0};
    };

    VulkanPipeline() = default;
    explicit VulkanPipeline(VulkanDevice& device, const CreateComputeInfo& computeInfo);
    explicit VulkanPipeline(VulkanDevice& device, const VulkanRenderPass& renderPass, const CreateInfo& createInfo);
    ~VulkanPipeline();
    VulkanPipeline(const VulkanPipeline& other) = delete;
    VulkanPipeline(VulkanPipeline&& other) noexcept;
    VulkanPipeline& operator=(const VulkanPipeline& other) = delete;
    VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;
    void swap(VulkanPipeline& other) noexcept;

    VkPipelineLayout& getLayout() {
        return pipelineLayout;
    }

    const VkPipelineLayout& getLayout() const {
        return pipelineLayout;
    }

    VkPipeline& getHandle() {
        return pipeline;
    }

    const VkPipeline& getHandle() const {
        return pipeline;
    }

    bool isCompute() const {
        return compute;
    }

    operator bool() const {
        return pipeline != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
    bool compute{false};
};
} // namespace Engine
