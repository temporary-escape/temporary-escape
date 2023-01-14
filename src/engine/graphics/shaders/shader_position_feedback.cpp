#include "shader_position_feedback.hpp"

using namespace Engine;

ShaderPositionFeedback::ShaderPositionFeedback(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules) {
    VkDescriptorSetLayoutBinding uboCameraLayoutBinding{};
    uboCameraLayoutBinding.binding = 0;
    uboCameraLayoutBinding.descriptorCount = 1;
    uboCameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCameraLayoutBinding.pImmutableSamplers = nullptr;
    uboCameraLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding sboInputLayoutBinding{};
    sboInputLayoutBinding.binding = 1;
    sboInputLayoutBinding.descriptorCount = 1;
    sboInputLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    sboInputLayoutBinding.pImmutableSamplers = nullptr;
    sboInputLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding sboOutputLayoutBinding{};
    sboOutputLayoutBinding.binding = 2;
    sboOutputLayoutBinding.descriptorCount = 1;
    sboOutputLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    sboOutputLayoutBinding.pImmutableSamplers = nullptr;
    sboOutputLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    descriptorSetLayout = vulkan.createDescriptorSetLayout({
        uboCameraLayoutBinding,
        sboInputLayoutBinding,
        sboOutputLayoutBinding,
    });

    auto& comp = modules.findByName("position-feedback.comp");

    VulkanPipeline::CreateComputeInfo pipelineInfo{};
    pipelineInfo.shaderModule = &comp;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(Uniforms);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.getHandle();
    pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    pipeline = vulkan.createPipeline(pipelineInfo);
}
