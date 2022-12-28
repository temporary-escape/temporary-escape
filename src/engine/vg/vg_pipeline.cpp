#include "vg_pipeline.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

VgPipeline::VgPipeline(const Config& config, VkDevice device, const VgRenderPass& renderPass,
                       const CreateInfo& createInfo) :
    device{device} {

    if (vkCreatePipelineLayout(device, &createInfo.pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create pipeline layout!");
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.resize(createInfo.shaderModules.size());

    for (size_t i = 0; i < shaderStages.size(); i++) {
        const auto& shaderModule = *createInfo.shaderModules.at(i);

        if (!shaderModule || shaderModule.getHandle() == VK_NULL_HANDLE) {
            destroy();
            EXCEPTION("Can not create pipeline with uninitialized shader module");
        }

        shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[i].stage = shaderModule.getStage();
        shaderStages[i].module = shaderModule.getHandle();
        shaderStages[i].pName = "main";
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &createInfo.vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &createInfo.inputAssembly;
    pipelineInfo.pViewportState = &createInfo.viewportState;
    pipelineInfo.pRasterizationState = &createInfo.rasterizer;
    pipelineInfo.pMultisampleState = &createInfo.multisampling;
    pipelineInfo.pColorBlendState = &createInfo.colorBlending;
    pipelineInfo.pDynamicState = &createInfo.dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass.getHandle();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        destroy();
        EXCEPTION("failed to create graphics pipeline!");
    }
}

VgPipeline::~VgPipeline() {
    destroy();
}

VgPipeline::VgPipeline(VgPipeline&& other) noexcept {
    swap(other);
}

VgPipeline& VgPipeline::operator=(VgPipeline&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgPipeline::swap(VgPipeline& other) noexcept {
    std::swap(device, other.device);
    std::swap(pipelineLayout, other.pipelineLayout);
    std::swap(pipeline, other.pipeline);
}

void VgPipeline::destroy() {
    if (pipelineLayout) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }

    if (pipeline) {
        vkDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
}
