#include "shader_component_point_cloud.hpp"

using namespace Engine;

ShaderComponentPointCloud::ShaderComponentPointCloud(const Config& config, VulkanRenderer& vulkan,
                                                     ShaderModules& modules, VulkanRenderPass& renderPass) {
    VkDescriptorSetLayoutBinding uboCameraLayoutBinding{};
    uboCameraLayoutBinding.binding = 0;
    uboCameraLayoutBinding.descriptorCount = 1;
    uboCameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCameraLayoutBinding.pImmutableSamplers = nullptr;
    uboCameraLayoutBinding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

    VkDescriptorSetLayoutBinding textureLayoutBinding{};
    textureLayoutBinding.binding = 1;
    textureLayoutBinding.descriptorCount = 1;
    textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureLayoutBinding.pImmutableSamplers = nullptr;
    textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    descriptorSetLayout = vulkan.createDescriptorSetLayout({
        uboCameraLayoutBinding,
        textureLayoutBinding,
    });

    auto& vert = modules.findByName("component-point-cloud.vert");
    auto& frag = modules.findByName("component-point-cloud.frag");
    auto& geom = modules.findByName("component-point-cloud.geom");

    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&vert, &frag, &geom};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, size);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, color);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, uv);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, st);

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    pipelineInfo.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    pipelineInfo.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
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
    pipelineInfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

    pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

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
    pushConstantRange.size = sizeof(Uniforms);
    pushConstantRange.stageFlags =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.getHandle();
    pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    pipelineInfo.depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineInfo.depthStencilState.depthTestEnable = VK_TRUE;
    pipelineInfo.depthStencilState.depthWriteEnable = VK_FALSE;
    pipelineInfo.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    pipelineInfo.depthStencilState.depthBoundsTestEnable = VK_FALSE;
    pipelineInfo.depthStencilState.stencilTestEnable = VK_FALSE;

    pipeline = vulkan.createPipeline(renderPass, pipelineInfo);
}
