#include "shader_brdf.hpp"

using namespace Engine;

ShaderBrdf::ShaderBrdf(const Config& config, VulkanRenderer& vulkan) : vulkan{&vulkan} {
    descriptorSetLayout = vulkan.createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>{});

    vert = vulkan.createShaderModule(config.shadersPath / "brdf.vert", VK_SHADER_STAGE_VERTEX_BIT);
    frag = vulkan.createShaderModule(config.shadersPath / "brdf.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void ShaderBrdf::finalize(VulkanRenderPass& renderPass) {
    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&vert, &frag};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    pipelineInfo.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    pipelineInfo.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

    pipelineInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineInfo.viewportState.viewportCount = 1;
    pipelineInfo.viewportState.scissorCount = 1;

    pipelineInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineInfo.rasterizer.depthClampEnable = VK_FALSE;
    pipelineInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    pipelineInfo.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineInfo.rasterizer.lineWidth = 1.0f;
    pipelineInfo.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineInfo.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

    pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    pipelineInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineInfo.colorBlending.logicOpEnable = VK_FALSE;
    pipelineInfo.colorBlending.logicOp = VK_LOGIC_OP_COPY;
    pipelineInfo.colorBlending.attachmentCount = 1;
    pipelineInfo.colorBlending.pAttachments = &colorBlendAttachment;
    pipelineInfo.colorBlending.blendConstants[0] = 0.0f;
    pipelineInfo.colorBlending.blendConstants[1] = 0.0f;
    pipelineInfo.colorBlending.blendConstants[2] = 0.0f;
    pipelineInfo.colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    pipelineInfo.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineInfo.dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    pipelineInfo.dynamicState.pDynamicStates = dynamicStates.data();

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int32_t);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.getHandle();
    pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    pipeline = vulkan->createPipeline(renderPass, pipelineInfo);
}