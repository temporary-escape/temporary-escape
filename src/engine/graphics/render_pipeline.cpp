#include "render_pipeline.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

static std::string idFromShaders(const std::vector<ShaderPtr>& shaders) {
    std::string id;
    for (const auto& shader : shaders) {
        if (!id.empty()) {
            id += ", ";
        }
        id += shader->getName();
    }
    return id;
}

RenderPipeline::RenderPipeline(VulkanRenderer& vulkan, std::vector<ShaderPtr> shaders, std::vector<VertexInput> inputs,
                               Options options) :
    vulkan{vulkan}, id{idFromShaders(shaders)}, createInfo{std::move(shaders), std::move(inputs), std::move(options)} {
}

RenderPipeline::RenderPipeline(VulkanRenderer& vulkan, std::vector<ShaderPtr> shaders) :
    vulkan{vulkan}, id{idFromShaders(shaders)}, createInfo{std::move(shaders), {}, {}} {
}

void RenderPipeline::init(VulkanRenderPass& renderPass, const std::vector<uint32_t>& attachments,
                          const uint32_t subpass) {
    logger.info("Creating graphics pipeline with shaders: '{}'", id);

    const auto resources = reflect();

    // Validate if the shader has any vertex inputs
    /*if (resources.vertexInputs.empty()) {
        EXCEPTION("Pipeline has no vertex inputs, shaders: '{}'", id);
    }*/

    createDescriptorSetLayout(resources);
    createPipeline(resources, renderPass, attachments, subpass);
    processPushConstants(resources);
}

void RenderPipeline::init() {
    logger.info("Creating compute pipeline with shaders: '{}'", id);

    const auto resources = reflect();

    createDescriptorSetLayout(resources);
    createPipeline(resources);
    processPushConstants(resources);
}

void RenderPipeline::processPushConstants(const ReflectInfo& resources) {
    for (const auto& field : resources.pushConstants.fields) {
        pushConstantsMap[field.name] = field;
    }
    pushConstantsSize = resources.pushConstants.size;

    for (const auto& uniform : resources.uniforms) {
        bindingsMap[uniform.second.name] = uniform.second.binding;
    }
    for (const auto& sampler : resources.samplers) {
        bindingsMap[sampler.name] = sampler.binding;
    }
    for (const auto& subpassInput : resources.subpassInputs) {
        bindingsMap[subpassInput.name] = subpassInput.binding;
    }
    for (const auto& storageBuffer : resources.storageBuffers) {
        bindingsMap[storageBuffer.name] = storageBuffer.binding;
    }
}

void RenderPipeline::pushConstantsBuffer(VulkanCommandBuffer& vkb, const char* src) {
    if (pushConstantsSize == 0) {
        EXCEPTION("Cannot push constants, pipeline: '{}' has no push constants", id);
    }

    auto flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
    if (pipeline.isCompute()) {
        flags = VK_SHADER_STAGE_COMPUTE_BIT;
    }

    vkb.pushConstants(getPipeline(), flags, 0, pushConstantsSize, src);
}

void RenderPipeline::bind(VulkanCommandBuffer& vkb) {
    vkb.bindPipeline(getPipeline());
}

uint32_t RenderPipeline::findBinding(const std::string_view& name) {
    const auto it = bindingsMap.find(name);
    if (it == bindingsMap.end()) {
        EXCEPTION("No such binding name: '{}' in pipeline: '{}'", name, id);
    }
    return it->second;
}

void RenderPipeline::bindDescriptors(VulkanCommandBuffer& vkb, const Span<UniformBindingRef>& uniforms,
                                     const Span<SamplerBindingRef>& textures,
                                     const Span<SubpassInputBindingRef>& inputs) {

    auto descriptorSet = getDescriptorPool().createDescriptorSet(descriptorSetLayout);

    size_t w{0};
    size_t b{0};
    size_t i{0};

    for (const auto& uniform : uniforms) {
        if (uniform.uniform->getDescriptorType() == VK_DESCRIPTOR_TYPE_MAX_ENUM) {
            EXCEPTION("Unable to use this buffer for descriptor set");
        }

        descriptors.buffers[b] = VkDescriptorBufferInfo{};
        descriptors.buffers[b].buffer = uniform.uniform->getHandle();
        descriptors.buffers[b].offset = 0;
        descriptors.buffers[b].range = uniform.uniform->getSize();

        descriptors.writes[w] = VkWriteDescriptorSet{};
        descriptors.writes[w].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptors.writes[w].dstSet = descriptorSet.getHandle();
        descriptors.writes[w].dstBinding = findBinding(uniform.name);
        descriptors.writes[w].dstArrayElement = 0;
        descriptors.writes[w].descriptorType = uniform.uniform->getDescriptorType();
        descriptors.writes[w].descriptorCount = 1;
        descriptors.writes[w].pBufferInfo = &descriptors.buffers[b];

        w++;
        b++;
    }

    for (const auto& texture : textures) {
        descriptors.images[i] = VkDescriptorImageInfo{};
        descriptors.images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors.images[i].imageView = texture.texture->getImageView();
        descriptors.images[i].sampler = texture.texture->getSampler();

        descriptors.writes[w] = VkWriteDescriptorSet{};
        descriptors.writes[w].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptors.writes[w].dstSet = descriptorSet.getHandle();
        descriptors.writes[w].dstBinding = findBinding(texture.name);
        descriptors.writes[w].dstArrayElement = 0;
        descriptors.writes[w].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptors.writes[w].descriptorCount = 1;
        descriptors.writes[w].pImageInfo = &descriptors.images[i];

        w++;
        i++;
    }

    for (const auto& input : inputs) {
        descriptors.images[i] = VkDescriptorImageInfo{};
        descriptors.images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors.images[i].imageView = input.texture->getImageView();
        descriptors.images[i].sampler = input.texture->getSampler();

        descriptors.writes[w] = VkWriteDescriptorSet{};
        descriptors.writes[w].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptors.writes[w].dstSet = descriptorSet.getHandle();
        descriptors.writes[w].dstBinding = findBinding(input.name);
        descriptors.writes[w].dstArrayElement = 0;
        descriptors.writes[w].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        descriptors.writes[w].descriptorCount = 1;
        descriptors.writes[w].pImageInfo = &descriptors.images[i];

        w++;
        i++;
    }

    descriptorSet.bind({descriptors.writes.data(), w});
    vkb.bindDescriptorSet(descriptorSet, pipeline.getLayout(), pipeline.isCompute());
}

RenderPipeline::ReflectInfo RenderPipeline::reflect() {
    ReflectInfo resources{};

    // Extract inputs and outputs from the shaders
    for (const auto& shader : createInfo.shaders) {
        const auto reflection = shader->getVulkanShader().reflect();

        if (shader->getStage() == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT) {
            resources.vertexInputs = reflection.getInputs();
        }

        if (shader->getStage() == VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT) {
            resources.samplers = reflection.getSamplers();
        }

        if (shader->getStage() == VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT) {
            resources.subpassInputs = reflection.getSubpassInputs();
        }

        if (shader->getStage() == VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT) {
            resources.storageBuffers = reflection.getStorageBuffers();
        }

        for (const auto& uniform : reflection.getUniforms()) {
            resources.uniforms[uniform.name] = uniform;
        }

        if (reflection.getPushConstants().size > 0) {
            if (resources.pushConstants.size > 0 && resources.pushConstants != reflection.getPushConstants()) {
                EXCEPTION("Inconsistent push constants, stages: {}", id);
            } else {
                resources.pushConstants = reflection.getPushConstants();
            }
        }
    }

    return resources;
}

void RenderPipeline::createDescriptorSetLayout(const ReflectInfo& resources) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};

    for (const auto& [_, uniform] : resources.uniforms) {
        layoutBindings.emplace_back();
        auto& binding = layoutBindings.back();

        binding.binding = uniform.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                             VK_SHADER_STAGE_COMPUTE_BIT;

        logger.debug("Adding uniform name: '{}' binding: {} size: {}", uniform.name, uniform.binding, uniform.size);
    }

    for (const auto& sampler : resources.samplers) {
        layoutBindings.emplace_back();
        auto& binding = layoutBindings.back();

        binding.binding = sampler.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        logger.debug("Adding sampler name: '{}' binding: {}", sampler.name, sampler.binding);
    }

    for (const auto& subpassInput : resources.subpassInputs) {
        layoutBindings.emplace_back();
        auto& binding = layoutBindings.back();

        binding.binding = subpassInput.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        logger.debug("Adding subpass input name: '{}' binding: {}", subpassInput.name, subpassInput.binding);
    }

    for (const auto& storageBuffer : resources.storageBuffers) {
        layoutBindings.emplace_back();
        auto& binding = layoutBindings.back();

        binding.binding = storageBuffer.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        logger.debug("Adding storage buffer name: '{}' binding: {}", storageBuffer.name, storageBuffer.binding);
    }

    descriptorSetLayout = vulkan.createDescriptorSetLayout(layoutBindings);

    createDescriptorPool(layoutBindings);
}

void RenderPipeline::createPipeline(const ReflectInfo& resources, VulkanRenderPass& renderPass,
                                    const std::vector<uint32_t>& attachments, const uint32_t subpass) {

    std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    for (const auto& input : createInfo.inputs) {
        bindingDescriptions.emplace_back();
        auto& binding = bindingDescriptions.back();

        if (input.size == 0 || input.layout.empty()) {
            EXCEPTION("Invalid vertex input layout for binding: {}", input.binding);
        }

        binding.binding = input.binding;
        binding.inputRate = input.inputRate;
        binding.stride = input.size;

        logger.debug("Adding vertex input binding: {} stride: {}", input.binding, input.size);

        for (const auto& attribute : input.layout) {
            attributeDescriptions.emplace_back();
            auto& description = attributeDescriptions.back();

            description.binding = input.binding;
            description.location = attribute.location;
            description.format = attribute.format;
            description.offset = attribute.offset;

            logger.debug("Adding vertex attribute binding: {} location: {} format: {} offset: {}",
                         input.binding,
                         attribute.location,
                         attribute.format,
                         attribute.offset);
        }
    }

    VulkanPipeline::CreateInfo pipelineInfo{};
    for (const auto& shader : createInfo.shaders) {
        pipelineInfo.shaderModules.push_back(&shader->getVulkanShader());
    }

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    pipelineInfo.vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    pipelineInfo.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInfo.inputAssembly.topology = createInfo.options.topology;
    pipelineInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

    pipelineInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineInfo.viewportState.viewportCount = 1;
    pipelineInfo.viewportState.scissorCount = 1;

    pipelineInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineInfo.rasterizer.depthClampEnable = VK_FALSE;
    pipelineInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    pipelineInfo.rasterizer.polygonMode = createInfo.options.polygonMode;
    pipelineInfo.rasterizer.lineWidth = 1.0f;
    pipelineInfo.rasterizer.cullMode = createInfo.options.cullMode;
    pipelineInfo.rasterizer.frontFace = createInfo.options.frontFace;
    pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

    pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
    for (const auto attachmentIndex : attachments) {
        if (attachmentIndex >= renderPass.getAttachments().size()) {
            EXCEPTION("Attachment: {} is out of bounds of render pass attachments", attachmentIndex);
        }

        const auto& attachment = renderPass.getAttachments()[attachmentIndex];
        if (isDepthFormat(attachment.format)) {
            continue;
        }

        colorBlendAttachments.emplace_back();
        auto& colorBlendAttachment = colorBlendAttachments.back();

        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        if (createInfo.options.blending == Blending::Normal) {
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        } else if (createInfo.options.blending == Blending::Additive) {
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        } else {
            colorBlendAttachment.blendEnable = VK_FALSE;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        }

        logger.debug(
            "Adding color blend attachment position: {} index: {}", colorBlendAttachments.size() - 1, attachmentIndex);
    }

    pipelineInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineInfo.colorBlending.logicOpEnable = VK_FALSE;
    pipelineInfo.colorBlending.logicOp = VK_LOGIC_OP_COPY;
    pipelineInfo.colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    pipelineInfo.colorBlending.pAttachments = colorBlendAttachments.data();
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
    pushConstantRange.size = resources.pushConstants.size;
    pushConstantRange.stageFlags =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.getHandle();

    if (pushConstantRange.size > 0) {
        logger.debug("Enabling push constants of size: {}", pushConstantRange.size);
        pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }

    pipelineInfo.depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    if (createInfo.options.depth == DepthMode::Read || createInfo.options.depth == DepthMode::ReadWrite) {
        logger.debug("Enabling depth test");
        pipelineInfo.depthStencilState.depthTestEnable = VK_TRUE;
    }
    if (createInfo.options.depth == DepthMode::Write || createInfo.options.depth == DepthMode::ReadWrite) {
        logger.debug("Enabling depth write");
        pipelineInfo.depthStencilState.depthWriteEnable = VK_TRUE;
    }
    pipelineInfo.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.depthStencilState.depthBoundsTestEnable = VK_FALSE;
    pipelineInfo.depthStencilState.stencilTestEnable = VK_FALSE;
    pipelineInfo.subpass = subpass;

    if (createInfo.options.stencil == Stencil::Write) {
        pipelineInfo.depthStencilState.stencilTestEnable = VK_TRUE;
        pipelineInfo.depthStencilState.back.failOp = VK_STENCIL_OP_REPLACE;
        pipelineInfo.depthStencilState.back.depthFailOp = VK_STENCIL_OP_REPLACE;
        pipelineInfo.depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;
        pipelineInfo.depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
        pipelineInfo.depthStencilState.back.compareMask = 0xff;
        pipelineInfo.depthStencilState.back.writeMask = 0xff;
        pipelineInfo.depthStencilState.back.reference = createInfo.options.stencilValue;
        pipelineInfo.depthStencilState.front = pipelineInfo.depthStencilState.back;
    } else if (createInfo.options.stencil == Stencil::Read) {
        pipelineInfo.depthStencilState.stencilTestEnable = VK_TRUE;
        pipelineInfo.depthStencilState.back.failOp = VK_STENCIL_OP_ZERO;
        pipelineInfo.depthStencilState.back.depthFailOp = VK_STENCIL_OP_ZERO;
        pipelineInfo.depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
        pipelineInfo.depthStencilState.back.compareOp = VK_COMPARE_OP_EQUAL;
        pipelineInfo.depthStencilState.back.compareMask = 0xff;
        pipelineInfo.depthStencilState.back.writeMask = 0xff;
        pipelineInfo.depthStencilState.back.reference = createInfo.options.stencilValue;
        pipelineInfo.depthStencilState.front = pipelineInfo.depthStencilState.back;
    }

    pipeline = vulkan.createPipeline(renderPass, pipelineInfo);
}

void RenderPipeline::createPipeline(const ReflectInfo& resources) {
    VulkanPipeline::CreateComputeInfo pipelineInfo{};

    pipelineInfo.shaderModule = &createInfo.shaders.back()->getVulkanShader();

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = resources.pushConstants.size;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    if (pushConstantRange.size > 0) {
        logger.debug("Enabling push constants of size: {}", pushConstantRange.size);
        pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.getHandle();

    pipeline = vulkan.createPipeline(pipelineInfo);
}

void RenderPipeline::createDescriptorPool(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings) {
    std::unordered_map<VkDescriptorType, uint32_t> descriptorTypeCounts;
    for (const auto& binding : layoutBindings) {
        descriptorTypeCounts[binding.descriptorType] += binding.descriptorCount;
    }

    for (auto& descriptorPool : descriptorPools) {
        VulkanDescriptorPool::CreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

        static const size_t maxSetsPerPool = 128;

        std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
        descriptorPoolSizes.resize(descriptorTypeCounts.size());
        size_t idx = 0;
        for (const auto& [type, count] : descriptorTypeCounts) {
            descriptorPoolSizes[idx].type = type;
            descriptorPoolSizes[idx].descriptorCount = count * maxSetsPerPool;

            logger.debug("Descriptor pool type: {} size: {}", type, descriptorPoolSizes[idx].descriptorCount);
            idx++;
        }

        poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
        poolInfo.pPoolSizes = descriptorPoolSizes.data();
        poolInfo.maxSets = maxSetsPerPool;

        descriptorPool = vulkan.createDescriptorPool(poolInfo);
    }
}

void RenderPipeline::renderMesh(VulkanCommandBuffer& vkb, const Mesh& mesh) {
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    if (mesh.vbo) {
        vboBindings[0] = {&mesh.vbo, 0};
        vkb.bindBuffers(vboBindings);
    }

    if (mesh.ibo) {
        vkb.bindIndexBuffer(mesh.ibo, 0, mesh.indexType);
        vkb.drawIndexed(mesh.count, mesh.instances, 0, 0, 0);
    } else {
        vkb.draw(mesh.count, mesh.instances, 0, 0);
    }
}
