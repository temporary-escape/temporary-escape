#pragma once

#include "../utils/path.hpp"
#include "vg_render_pass.hpp"
#include "vg_shader_module.hpp"

namespace Engine {
class VgPipeline {
public:
    struct CreateInfo {
        std::vector<VgShaderModule*> shaderModules;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    };

    VgPipeline() = default;
    explicit VgPipeline(const Config& config, VkDevice device, const VgRenderPass& renderPass,
                        const CreateInfo& createInfo);
    ~VgPipeline();
    VgPipeline(const VgPipeline& other) = delete;
    VgPipeline(VgPipeline&& other) noexcept;
    VgPipeline& operator=(const VgPipeline& other) = delete;
    VgPipeline& operator=(VgPipeline&& other) noexcept;
    void swap(VgPipeline& other) noexcept;

    VkPipelineLayout getLayout() const {
        return pipelineLayout;
    }

    VkPipeline getHandle() const {
        return pipeline;
    }

    operator bool() const {
        return pipeline != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
};
} // namespace Engine
