#pragma once

#include "../utils/path.hpp"
#include "vg_render_pass.hpp"
#include "vg_shader_module.hpp"

namespace Engine {
class VgDevice;

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
    explicit VgPipeline(const Config& config, VgDevice& device, const VgRenderPass& renderPass,
                        const CreateInfo& createInfo);
    ~VgPipeline();
    VgPipeline(const VgPipeline& other) = delete;
    VgPipeline(VgPipeline&& other) noexcept;
    VgPipeline& operator=(const VgPipeline& other) = delete;
    VgPipeline& operator=(VgPipeline&& other) noexcept;
    void swap(VgPipeline& other) noexcept;

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

    operator bool() const {
        return pipeline != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VgDevice* device{nullptr};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
};
} // namespace Engine
