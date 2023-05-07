#include "skybox_generator.hpp"
#include "../assets/registry.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

static const std::array<Matrix4, 6> captureViewMatrices = {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
};

// Simple skybox box with two triangles per side.
static const std::vector<float> skyboxVertices = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
};

static const float PI = static_cast<float>(std::atan(1) * 4);

// The projection matrix that will be used to generate the skybox.
// This must be 90 degrees view (PI/2)
static const glm::mat4 captureProjectionMatrix = glm::perspective(PI / 2.0, 1.0, 0.1, 1000.0);

// For some reason the first side (prefilter) texture is always bad no matter what I do.
// Temporary workaround, render the first side one more time. This seems to work.
static const std::array<int, 6> sidesToRender = {0, 1, 2, 3, 4, 5};

SkyboxGenerator::SkyboxGenerator(const Config& config, VulkanRenderer& vulkan, Registry& registry) :
    config{config}, vulkan{vulkan} {

    createSkyboxMesh();

    createRenderPass(renderPasses.color,
                     Vector2i{config.graphics.skyboxSize},
                     VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_ATTACHMENT_LOAD_OP_CLEAR);
    createRenderPass(renderPasses.irradiance,
                     Vector2i{config.graphics.skyboxIrradianceSize},
                     VK_FORMAT_R16G16B16A16_SFLOAT,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    createRenderPass(renderPasses.prefilter,
                     Vector2i{config.graphics.skyboxPrefilterSize},
                     VK_FORMAT_R16G16B16A16_SFLOAT,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    //createPipelineStars(registry, renderPasses.color.renderPass);
    createPipelineNebula(registry, renderPasses.color.renderPass);
    createPipelinePrefilter(registry, renderPasses.prefilter.renderPass);
    createPipelineIrradiance(registry, renderPasses.irradiance.renderPass);
}

void SkyboxGenerator::createPipelinePrefilter(Registry& registry, VulkanRenderPass& renderPass) {
    VkDescriptorSetLayoutBinding uboCameraLayoutBinding{};
    uboCameraLayoutBinding.binding = 0;
    uboCameraLayoutBinding.descriptorCount = 1;
    uboCameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCameraLayoutBinding.pImmutableSamplers = nullptr;
    uboCameraLayoutBinding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

    VkDescriptorSetLayoutBinding skyboxTextureLayoutBinding{};
    skyboxTextureLayoutBinding.binding = 1;
    skyboxTextureLayoutBinding.descriptorCount = 1;
    skyboxTextureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    skyboxTextureLayoutBinding.pImmutableSamplers = nullptr;
    skyboxTextureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    shaders.prefilter.descriptorSetLayout = vulkan.createDescriptorSetLayout({
        uboCameraLayoutBinding,
        skyboxTextureLayoutBinding,
    });

    auto vert = registry.getShaders().find("skybox-prefilter.vert");
    auto frag = registry.getShaders().find("skybox-prefilter.frag");

    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&vert->getVulkanShader(), &frag->getVulkanShader()};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vector3);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescription{};

    attributeDescription.binding = 0;
    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = 0;

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    pipelineInfo.vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

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
    pipelineInfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

    pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
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
    pushConstantRange.size = sizeof(SkyboxPrefilterUniforms);
    pushConstantRange.stageFlags =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &shaders.prefilter.descriptorSetLayout.getHandle();
    pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    shaders.prefilter.pipeline = vulkan.createPipeline(renderPass, pipelineInfo);
}

/*void SkyboxGenerator::createPipelineStars(Registry& registry, VulkanRenderPass& renderPass) {
    VkDescriptorSetLayoutBinding uboCameraLayoutBinding{};
    uboCameraLayoutBinding.binding = 0;
    uboCameraLayoutBinding.descriptorCount = 1;
    uboCameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCameraLayoutBinding.pImmutableSamplers = nullptr;
    uboCameraLayoutBinding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

    shaders.stars.descriptorSetLayout = vulkan.createDescriptorSetLayout({
        uboCameraLayoutBinding,
    });

    auto vert = registry.getShaders().find("skybox-stars.vert");
    auto frag = registry.getShaders().find("skybox-stars.frag");
    auto geom = registry.getShaders().find("skybox-stars.geom");

    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&vert->getVulkanShader(), &frag->getVulkanShader(), &geom->getVulkanShader()};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(StarVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(StarVertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(StarVertex, brightness);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(StarVertex, color);

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
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

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
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
    pushConstantRange.size = sizeof(SkyboxStarsUniforms);
    pushConstantRange.stageFlags =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &shaders.stars.descriptorSetLayout.getHandle();
    pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    shaders.stars.pipeline = vulkan.createPipeline(renderPass, pipelineInfo);
}*/

void SkyboxGenerator::createPipelineNebula(Registry& registry, VulkanRenderPass& renderPass) {
    VkDescriptorSetLayoutBinding uboCameraLayoutBinding{};
    uboCameraLayoutBinding.binding = 0;
    uboCameraLayoutBinding.descriptorCount = 1;
    uboCameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCameraLayoutBinding.pImmutableSamplers = nullptr;
    uboCameraLayoutBinding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

    shaders.nebula.descriptorSetLayout = vulkan.createDescriptorSetLayout({
        uboCameraLayoutBinding,
    });

    auto vert = registry.getShaders().find("skybox-nebula.vert");
    auto frag = registry.getShaders().find("skybox-nebula.frag");

    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&vert->getVulkanShader(), &frag->getVulkanShader()};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vector3);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescription{};

    attributeDescription.binding = 0;
    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = 0;

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    pipelineInfo.vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

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
    pipelineInfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

    pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
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
    pushConstantRange.size = sizeof(SkyboxNebulaUniforms);
    pushConstantRange.stageFlags =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &shaders.nebula.descriptorSetLayout.getHandle();
    pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    shaders.nebula.pipeline = vulkan.createPipeline(renderPass, pipelineInfo);
}

void SkyboxGenerator::createPipelineIrradiance(Registry& registry, VulkanRenderPass& renderPass) {
    VkDescriptorSetLayoutBinding uboCameraLayoutBinding{};
    uboCameraLayoutBinding.binding = 0;
    uboCameraLayoutBinding.descriptorCount = 1;
    uboCameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCameraLayoutBinding.pImmutableSamplers = nullptr;
    uboCameraLayoutBinding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

    VkDescriptorSetLayoutBinding skyboxTextureLayoutBinding{};
    skyboxTextureLayoutBinding.binding = 1;
    skyboxTextureLayoutBinding.descriptorCount = 1;
    skyboxTextureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    skyboxTextureLayoutBinding.pImmutableSamplers = nullptr;
    skyboxTextureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    shaders.irradiance.descriptorSetLayout = vulkan.createDescriptorSetLayout({
        uboCameraLayoutBinding,
        skyboxTextureLayoutBinding,
    });

    auto vert = registry.getShaders().find("skybox-irradiance.vert");
    auto frag = registry.getShaders().find("skybox-irradiance.frag");

    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&vert->getVulkanShader(), &frag->getVulkanShader()};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vector3);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescription{};

    attributeDescription.binding = 0;
    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = 0;

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = 1;
    pipelineInfo.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    pipelineInfo.vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

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
    pipelineInfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

    pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
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

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &shaders.irradiance.descriptorSetLayout.getHandle();

    shaders.irradiance.pipeline = vulkan.createPipeline(renderPass, pipelineInfo);
}

void SkyboxGenerator::createSkyboxMesh() {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(float) * skyboxVertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, skyboxVertices.data(), sizeof(float) * skyboxVertices.size());
}

void SkyboxGenerator::createAttachment(RenderPass& renderPass, const Vector2i& size, const VkFormat format) {
    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = format;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = format;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = 0.0f;

    renderPass.texture = vulkan.createTexture(textureInfo);
}

void SkyboxGenerator::createFramebuffer(RenderPass& renderPass, const Vector2i& size) {
    std::array<VkImageView, 1> attachmentViews = {
        renderPass.texture.getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = size.x;
    framebufferInfo.height = size.y;
    framebufferInfo.layers = 1;

    renderPass.fbo = vulkan.createFramebuffer(framebufferInfo);
}

void SkyboxGenerator::createRenderPass(RenderPass& renderPass, const Vector2i& size, const VkFormat format,
                                       const VkImageLayout finalLayout, const VkAttachmentLoadOp loadOp) {
    createAttachment(renderPass, size, format);

    std::vector<VkAttachmentDescription> attachments{1};
    attachments[0].format = format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = loadOp; // VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = finalLayout; // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 1> colorAttachmentRefs{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPass.renderPass = vulkan.createRenderPass(renderPassInfo);

    createFramebuffer(renderPass, size);
}

static void transitionTexture(VulkanCommandBuffer& vkb, VulkanTexture& texture, const VkImageLayout oldLayout,
                              const VkImageLayout newLayout, const VkPipelineStageFlags srcStageMask,
                              const VkPipelineStageFlags dstStageMask) {

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = srcStageMask;
    barrier.dstAccessMask = dstStageMask;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkb.pipelineBarrier(sourceStage, destinationStage, barrier);
}

static void copyTexture(VulkanCommandBuffer& vkb, VulkanTexture& source, VulkanTexture& target, int side) {
    VkOffset3D offset = {
        static_cast<int32_t>(source.getExtent().width),
        static_cast<int32_t>(source.getExtent().height),
        1,
    };

    std::array<VkImageBlit, 1> blit{};
    blit[0].srcOffsets[0] = {0, 0, 0};
    blit[0].srcOffsets[1] = offset;
    blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].srcSubresource.mipLevel = 0;
    blit[0].srcSubresource.baseArrayLayer = 0;
    blit[0].srcSubresource.layerCount = 1;
    blit[0].dstOffsets[0] = {0, 0, 0};
    blit[0].dstOffsets[1] = offset;
    blit[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].dstSubresource.mipLevel = 0;
    blit[0].dstSubresource.baseArrayLayer = side;
    blit[0].dstSubresource.layerCount = 1;

    vkb.blitImage(source,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  target,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  blit,
                  VK_FILTER_NEAREST);
}

Skybox SkyboxGenerator::generate(uint64_t seed) {
    logger.info("Generating skybox with seed: {}", seed);

    Rng rng{seed};

    Skybox skybox{};

    Stars starsSmall{};
    Stars starsLarge{};
    prepareStars(rng, starsSmall, 50000);
    prepareStars(rng, starsLarge, 1000);

    std::vector<SkyboxNebulaUniforms> nebulaParams{};
    prepareNebulaUbo(rng, nebulaParams);

    prepareCubemap(skybox);

    // Render all sides
    for (auto side : sidesToRender) {
        prepareCameraUbo(side);

        auto vkb = vulkan.createCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkb.start(beginInfo);

        // Transition the FBO texture so we can copy from it
        transitionTexture(vkb,
                          renderPasses.color.texture,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          0,
                          VK_ACCESS_SHADER_WRITE_BIT);

        // Render pass for creating the skybox
        VulkanRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.framebuffer = &renderPasses.color.fbo;
        renderPassInfo.renderPass = &renderPasses.color.renderPass;
        renderPassInfo.offset = {0, 0};
        renderPassInfo.size = {config.graphics.skyboxSize, config.graphics.skyboxSize};
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValues = {clearColor};
        vkb.beginRenderPass(renderPassInfo);
        vkb.setScissor({0, 0}, renderPassInfo.size);
        vkb.setViewport({0, 0}, renderPassInfo.size);

        //renderStars(vkb, starsSmall, 0.10f);
        //renderStars(vkb, starsLarge, 0.20f);

        for (const auto& p : nebulaParams) {
            renderNebula(vkb, p);
        }

        vkb.endRenderPass();

        // Copy the color texture to the destination
        copyTexture(vkb, renderPasses.color.texture, skybox.getTexture(), side);

        // Wait for the commands to finish
        vkb.end();
        vulkan.submitCommandBuffer(vkb);
        vulkan.waitQueueIdle();
        vulkan.dispose(std::move(vkb));
    }

    vulkan.generateMipMaps(skybox.getTexture());

    // Render all sides
    for (auto side : sidesToRender) {
        prepareCameraUbo(side);

        auto vkb = vulkan.createCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkb.start(beginInfo);

        // Render pass for creating an irradiance texture
        VulkanRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.framebuffer = &renderPasses.irradiance.fbo;
        renderPassInfo.renderPass = &renderPasses.irradiance.renderPass;
        renderPassInfo.offset = {0, 0};
        renderPassInfo.size = {config.graphics.skyboxIrradianceSize, config.graphics.skyboxIrradianceSize};
        renderPassInfo.clearValues = {VkClearValue{}};
        vkb.beginRenderPass(renderPassInfo);
        vkb.setScissor({0, 0}, renderPassInfo.size);
        vkb.setViewport({0, 0}, renderPassInfo.size);

        renderIrradiance(vkb, skybox.getTexture());

        vkb.endRenderPass();

        // Render pass for creating an prefilter texture
        renderPassInfo = VulkanRenderPassBeginInfo{};
        renderPassInfo.framebuffer = &renderPasses.prefilter.fbo;
        renderPassInfo.renderPass = &renderPasses.prefilter.renderPass;
        renderPassInfo.offset = {0, 0};
        renderPassInfo.size = {config.graphics.skyboxPrefilterSize, config.graphics.skyboxPrefilterSize};
        renderPassInfo.clearValues = {VkClearValue{}};
        vkb.beginRenderPass(renderPassInfo);
        vkb.setScissor({0, 0}, renderPassInfo.size);
        vkb.setViewport({0, 0}, renderPassInfo.size);

        renderPrefilter(vkb, skybox.getTexture(), renderPassInfo.size, 0);

        vkb.endRenderPass();

        // Copy the irradiance and prefilter textures to the destinations
        copyTexture(vkb, renderPasses.irradiance.texture, skybox.getIrradiance(), side);
        copyTexture(vkb, renderPasses.prefilter.texture, skybox.getPrefilter(), side);

        // Wait for the commands to finish
        vkb.end();
        vulkan.submitCommandBuffer(vkb);
        vulkan.waitQueueIdle();
        vulkan.dispose(std::move(vkb));
    }

    vulkan.generateMipMaps(skybox.getIrradiance());
    vulkan.generateMipMaps(skybox.getPrefilter());

    return skybox;
}

void SkyboxGenerator::prepareCameraUbo(int side) {
    CameraUniform uniform{};
    uniform.viewMatrix = captureViewMatrices[side];
    uniform.projectionMatrix = captureProjectionMatrix;

    if (!renderPasses.cameraUbo) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(CameraUniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        renderPasses.cameraUbo = vulkan.createBuffer(bufferInfo);
    }

    renderPasses.cameraUbo.subDataLocal(&uniform, 0, sizeof(CameraUniform));
}

/*void SkyboxGenerator::renderStars(VulkanCommandBuffer& vkb, SkyboxGenerator::Stars& stars, const float particleSize) {
    vkb.bindPipeline(shaders.stars.pipeline);

    std::array<VulkanVertexBufferBindRef, 1> vboBinings{};
    vboBinings[0] = {&stars.vbo, 0};
    vkb.bindBuffers(vboBinings);

    SkyboxStarsUniforms constants{};
    constants.particleSize = {particleSize, particleSize};
    vkb.pushConstants(shaders.stars.pipeline,
                      VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                      0,
                      sizeof(SkyboxStarsUniforms),
                      &constants);

    std::array<VulkanBufferBinding, 1> uboBindings{};
    uboBindings[0] = {0, &renderPasses.cameraUbo};

    vkb.bindDescriptors(shaders.stars.pipeline, shaders.stars.descriptorSetLayout, uboBindings, {}, {});

    vkb.draw(stars.count, 1, 0, 0);
}*/

void SkyboxGenerator::renderNebula(VulkanCommandBuffer& vkb, const SkyboxNebulaUniforms& params) {
    vkb.bindPipeline(shaders.nebula.pipeline);

    std::array<VulkanVertexBufferBindRef, 1> vboBinings{};
    vboBinings[0] = {&mesh.vbo, 0};
    vkb.bindBuffers(vboBinings);

    vkb.pushConstants(shaders.nebula.pipeline,
                      VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                      0,
                      sizeof(SkyboxNebulaUniforms),
                      &params);

    std::array<VulkanBufferBinding, 1> uboBindings{};
    uboBindings[0] = {0, &renderPasses.cameraUbo};

    vkb.bindDescriptors(shaders.nebula.pipeline, shaders.nebula.descriptorSetLayout, uboBindings, {}, {});

    vkb.draw(skyboxVertices.size(), 1, 0, 0);
}

void SkyboxGenerator::renderIrradiance(VulkanCommandBuffer& vkb, VulkanTexture& color) {
    vkb.bindPipeline(shaders.irradiance.pipeline);

    std::array<VulkanVertexBufferBindRef, 1> vboBinings{};
    vboBinings[0] = {&mesh.vbo, 0};
    vkb.bindBuffers(vboBinings);

    std::array<VulkanBufferBinding, 1> uboBindings{};
    uboBindings[0] = {0, &renderPasses.cameraUbo};

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {1, &color};

    vkb.bindDescriptors(
        shaders.irradiance.pipeline, shaders.irradiance.descriptorSetLayout, uboBindings, textureBindings, {});

    vkb.draw(skyboxVertices.size(), 1, 0, 0);
}

void SkyboxGenerator::renderPrefilter(VulkanCommandBuffer& vkb, VulkanTexture& color, const Vector2i& viewport,
                                      int level) {
    vkb.bindPipeline(shaders.prefilter.pipeline);

    std::array<VulkanVertexBufferBindRef, 1> vboBinings{};
    vboBinings[0] = {&mesh.vbo, 0};
    vkb.bindBuffers(vboBinings);

    const auto mipmaps = static_cast<int>(std::log2(viewport.x));
    const auto roughness = static_cast<float>(level) / static_cast<float>(mipmaps - 1);

    SkyboxPrefilterUniforms uniforms{};
    uniforms.roughness = roughness;

    vkb.pushConstants(shaders.prefilter.pipeline,
                      VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                      0,
                      sizeof(SkyboxPrefilterUniforms),
                      &uniforms);

    std::array<VulkanBufferBinding, 1> uboBindings{};
    uboBindings[0] = {0, &renderPasses.cameraUbo};

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {1, &color};

    vkb.bindDescriptors(
        shaders.prefilter.pipeline, shaders.prefilter.descriptorSetLayout, uboBindings, textureBindings, {});

    vkb.draw(skyboxVertices.size(), 1, 0, 0);
}

void SkyboxGenerator::prepareCubemap(Skybox& skybox) {
    auto size = Vector2i{config.graphics.skyboxSize, config.graphics.skyboxSize};

    logger.debug("Preparing skybox cubemap of size: {}", size);

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(size);
    textureInfo.image.arrayLayers = 6;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    textureInfo.image.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 6;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    skybox.getTexture() = vulkan.createTexture(textureInfo);

    size = Vector2i{config.graphics.skyboxPrefilterSize, config.graphics.skyboxPrefilterSize};

    textureInfo.image.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.view.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(size);
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    skybox.getPrefilter() = vulkan.createTexture(textureInfo);

    size = Vector2i{config.graphics.skyboxIrradianceSize, config.graphics.skyboxIrradianceSize};

    textureInfo.image.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.view.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(size);
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    skybox.getIrradiance() = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(skybox.getTexture(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.transitionImageLayout(
        skybox.getIrradiance(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.transitionImageLayout(
        skybox.getPrefilter(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

void SkyboxGenerator::prepareStars(Rng& rng, Stars& stars, const size_t count) {
    logger.debug("Preparing skybox vertex buffer for: {} stars", count);

    std::uniform_real_distribution<float> distPosition(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distColor(0.9f, 1.0f);
    std::uniform_real_distribution<float> distBrightness(0.5f, 1.0f);

    std::vector<StarVertex> vertices{count};

    for (auto& star : vertices) {
        star.position = glm::normalize(Vector3{distPosition(rng), distPosition(rng), distPosition(rng)}) * 100.0f;
        star.color = Color4{distColor(rng), distColor(rng), distColor(rng), 1.0f};
        star.brightness = distBrightness(rng);
    }

    stars.count = count;

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(StarVertex) * count;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    stars.vbo = vulkan.createBuffer(bufferInfo);
    stars.vbo.subDataLocal(vertices.data(), 0, bufferInfo.size);
}

void SkyboxGenerator::prepareNebulaUbo(Rng& rng, std::vector<SkyboxNebulaUniforms>& params) {
    std::uniform_real_distribution<float> dist{0.0f, 1.0f};

    while (true) {
        const auto scale = dist(rng) * 0.5f + 0.25f;
        const auto intensity = dist(rng) * 0.2f + 0.9f;
        const auto color = Color4{dist(rng), dist(rng), dist(rng), 1.0f};
        const auto falloff = dist(rng) * 3.0f + 3.0f;
        const auto offset =
            Vector3{dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f};

        SkyboxNebulaUniforms uniforms{};
        uniforms.uColor = color;
        uniforms.uOffset = Vector4{offset, 1.0f};
        uniforms.uScale = scale;
        uniforms.uIntensity = intensity;
        uniforms.uFalloff = falloff;

        params.push_back(uniforms);

        if (dist(rng) < 0.5f) {
            break;
        }
    }
}
