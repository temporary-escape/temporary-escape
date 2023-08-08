#include "render_pipeline.hpp"

using namespace Engine;

RenderPipeline::RenderPipeline(VulkanRenderer& vulkan, std::string name) : vulkan{vulkan}, name{std::move(name)} {
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipeline::create(VulkanRenderPass& renderPass, const uint32_t subpass,
                            const std::vector<uint32_t>& attachments) {
    const auto resources = reflect();

    createDescriptorSetLayout(resources);
    if (compute) {
        createComputePipeline(resources);
    } else {
        createGraphicsPipeline(renderPass, resources, subpass, attachments);
    }
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

uint32_t RenderPipeline::findBinding(const std::string_view& binding) {
    const auto it = bindingsMap.find(binding);
    if (it == bindingsMap.end()) {
        EXCEPTION("No such binding name: '{}' in pipeline: '{}'", binding, name);
    }
    return it->second;
}

void RenderPipeline::bindDescriptors(VulkanCommandBuffer& vkb, const Span<UniformBindingRef>& uniforms,
                                     const Span<SamplerBindingRef>& textures,
                                     const Span<SubpassInputBindingRef>& inputs) {

    auto descriptorSet = descriptorPools.at(vulkan.getCurrentFrameNum()).createDescriptorSet(descriptorSetLayout);

    size_t w{0};
    size_t b{0};
    size_t i{0};

    for (const auto& uniform : uniforms) {
        if (uniform.uniform->getDescriptorType() == VK_DESCRIPTOR_TYPE_MAX_ENUM) {
            EXCEPTION("Unable to use this buffer for descriptor set");
        }

        descriptors.buffers[b] = VkDescriptorBufferInfo{};
        descriptors.buffers[b].buffer = uniform.uniform->getHandle();
        descriptors.buffers[b].offset = uniform.offset;
        descriptors.buffers[b].range = uniform.range;

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

void RenderPipeline::pushConstantsBuffer(VulkanCommandBuffer& vkb, const char* src) {
    if (pushConstantsSize == 0) {
        EXCEPTION("Cannot push constants, pipeline: '{}' has no push constants", name);
    }

    auto flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
    if (pipeline.isCompute()) {
        flags = VK_SHADER_STAGE_COMPUTE_BIT;
    }

    vkb.pushConstants(pipeline, flags, 0, pushConstantsSize, src);
}

RenderPipeline::ReflectInfo RenderPipeline::reflect() const {
    ReflectInfo resources{};

    // Extract inputs and outputs from the shaders
    for (const auto& shader : shaders) {
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
                EXCEPTION("Inconsistent push constants shader: {}", shader->getName());
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
    }

    for (const auto& sampler : resources.samplers) {
        layoutBindings.emplace_back();
        auto& binding = layoutBindings.back();

        binding.binding = sampler.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    for (const auto& subpassInput : resources.subpassInputs) {
        layoutBindings.emplace_back();
        auto& binding = layoutBindings.back();

        binding.binding = subpassInput.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    for (const auto& storageBuffer : resources.storageBuffers) {
        layoutBindings.emplace_back();
        auto& binding = layoutBindings.back();

        binding.binding = storageBuffer.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }

    descriptorSetLayout = vulkan.createDescriptorSetLayout(layoutBindings);

    createDescriptorPool(layoutBindings);
}

void RenderPipeline::createDescriptorPool(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings) {
    std::unordered_map<VkDescriptorType, uint32_t> descriptorTypeCounts;
    for (const auto& binding : layoutBindings) {
        descriptorTypeCounts[binding.descriptorType] += binding.descriptorCount;
    }

    VulkanDescriptorPool::CreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    static const size_t maxSetsPerPool = 128;

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
    descriptorPoolSizes.resize(descriptorTypeCounts.size());
    size_t idx = 0;
    for (const auto& [type, count] : descriptorTypeCounts) {
        descriptorPoolSizes[idx].type = type;
        descriptorPoolSizes[idx].descriptorCount = count * maxSetsPerPool;

        idx++;
    }

    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = maxSetsPerPool;

    for (auto& descriptorPool : descriptorPools) {
        descriptorPool = vulkan.createDescriptorPool(poolInfo);
    }
}

void RenderPipeline::createGraphicsPipeline(VulkanRenderPass& renderPass, const ReflectInfo& resources,
                                            const uint32_t subpass, const std::vector<uint32_t>& attachments) {
    if (shaders.empty()) {
        EXCEPTION("Can not create graphics pipeline with no shaders");
    }

    pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    pipelineInfo.vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    pipelineInfo.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

    pipelineInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineInfo.viewportState.viewportCount = 1;
    pipelineInfo.viewportState.scissorCount = 1;

    pipelineInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    pipelineInfo.rasterizer.lineWidth = 1.0f;
    pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

    pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
    for (size_t attachmentIndex = 0; attachmentIndex < attachments.size(); attachmentIndex++) {
        const auto attachment = attachments.at(attachmentIndex);

        if (attachmentIndex >= renderPass.getAttachments().size()) {
            EXCEPTION("Attachment: {} is out of bounds of render pass attachments", attachmentIndex);
        }

        const auto& attachmentDescription = renderPass.getAttachments()[attachmentIndex];
        if (isDepthFormat(attachmentDescription.format)) {
            continue;
        }

        colorBlendAttachments.emplace_back();
        auto& colorBlendAttachment = colorBlendAttachments.back();

        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        if (attachmentBlending == Blending::Normal) {
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        } else if (attachmentBlending == Blending::Additive) {
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
        pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }

    pipelineInfo.depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineInfo.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.depthStencilState.depthBoundsTestEnable = VK_FALSE;
    pipelineInfo.subpass = subpass;

    pipeline = vulkan.createPipeline(renderPass, pipelineInfo);
}

void RenderPipeline::createComputePipeline(const RenderPipeline::ReflectInfo& resources) {
    if (shaders.empty()) {
        EXCEPTION("Can not create compute pipeline with no shaders");
    }

    VulkanPipeline::CreateComputeInfo computeInfo{};

    computeInfo.shaderModule = &shaders.back()->getVulkanShader();

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = resources.pushConstants.size;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    if (pushConstantRange.size > 0) {
        computeInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
        computeInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }

    computeInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeInfo.pipelineLayoutInfo.setLayoutCount = 1;
    computeInfo.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.getHandle();

    pipeline = vulkan.createPipeline(computeInfo);
}

void RenderPipeline::addShader(const ShaderPtr& shader) {
    shaders.push_back(shader);
    pipelineInfo.shaderModules.push_back(&shader->getVulkanShader());
}

void RenderPipeline::addVertexInput(const VertexInput& input) {
    bindingDescriptions.emplace_back();
    auto& binding = bindingDescriptions.back();

    if (input.size == 0 || input.layout.empty()) {
        EXCEPTION("Invalid vertex input layout for binding: {}", input.binding);
    }

    binding.binding = input.binding;
    binding.inputRate = input.inputRate;
    binding.stride = input.size;

    for (const auto& attribute : input.layout) {
        attributeDescriptions.emplace_back();
        auto& description = attributeDescriptions.back();

        description.binding = input.binding;
        description.location = attribute.location;
        description.format = attribute.format;
        description.offset = attribute.offset;
    }
}

void RenderPipeline::setTopology(const VkPrimitiveTopology topology) {
    pipelineInfo.inputAssembly.topology = topology;
}

void RenderPipeline::setPolygonMode(const VkPolygonMode polygonMode) {
    pipelineInfo.rasterizer.polygonMode = polygonMode;
}

void RenderPipeline::setCullMode(const VkCullModeFlags cullMode) {
    pipelineInfo.rasterizer.cullMode = cullMode;
}

void RenderPipeline::setFrontFace(const VkFrontFace frontFace) {
    pipelineInfo.rasterizer.frontFace = frontFace;
}

void RenderPipeline::setDepthMode(const RenderPipeline::DepthMode depthMode) {
    if (depthMode == DepthMode::Read || depthMode == DepthMode::ReadWrite) {
        pipelineInfo.depthStencilState.depthTestEnable = VK_TRUE;
    } else {
        pipelineInfo.depthStencilState.depthTestEnable = VK_FALSE;
    }

    if (depthMode == DepthMode::Write || depthMode == DepthMode::ReadWrite) {
        pipelineInfo.depthStencilState.depthWriteEnable = VK_TRUE;
    } else {
        pipelineInfo.depthStencilState.depthWriteEnable = VK_FALSE;
    }
}

void RenderPipeline::setStencil(const RenderPipeline::Stencil stencil, const int stencilValue) {
    if (stencil == Stencil::Write) {
        pipelineInfo.depthStencilState.stencilTestEnable = VK_TRUE;
        pipelineInfo.depthStencilState.back.failOp = VK_STENCIL_OP_REPLACE;
        pipelineInfo.depthStencilState.back.depthFailOp = VK_STENCIL_OP_REPLACE;
        pipelineInfo.depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;
        pipelineInfo.depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
        pipelineInfo.depthStencilState.back.compareMask = 0xff;
        pipelineInfo.depthStencilState.back.writeMask = 0xff;
        pipelineInfo.depthStencilState.back.reference = stencilValue;
        pipelineInfo.depthStencilState.front = pipelineInfo.depthStencilState.back;
    } else if (stencil == Stencil::Read) {
        pipelineInfo.depthStencilState.stencilTestEnable = VK_TRUE;
        pipelineInfo.depthStencilState.back.failOp = VK_STENCIL_OP_ZERO;
        pipelineInfo.depthStencilState.back.depthFailOp = VK_STENCIL_OP_ZERO;
        pipelineInfo.depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
        pipelineInfo.depthStencilState.back.compareOp = VK_COMPARE_OP_EQUAL;
        pipelineInfo.depthStencilState.back.compareMask = 0xff;
        pipelineInfo.depthStencilState.back.writeMask = 0xff;
        pipelineInfo.depthStencilState.back.reference = stencilValue;
        pipelineInfo.depthStencilState.front = pipelineInfo.depthStencilState.back;
    } else {
        pipelineInfo.depthStencilState.stencilTestEnable = VK_FALSE;
    }
}

void RenderPipeline::setDepthClamp(const DepthClamp depthClamp) {
    if (depthClamp == DepthClamp::Enabled && vulkan.getPhysicalDeviceFeatures().depthClamp) {
        pipelineInfo.rasterizer.depthClampEnable = VK_TRUE;
    } else {
        pipelineInfo.rasterizer.depthClampEnable = VK_FALSE;
    }
}

void RenderPipeline::setCompute(const bool value) {
    compute = value;
}

void RenderPipeline::setBlending(const RenderPipeline::Blending blending) {
    attachmentBlending = blending;
}

VulkanDescriptorPool& RenderPipeline::getDescriptionPool() {
    return descriptorPools.at(vulkan.getCurrentFrameNum());
}

void RenderPipeline::bind(VulkanCommandBuffer& vkb) {
    vkb.bindPipeline(pipeline);
}

void RenderPipeline::renderMesh(VulkanCommandBuffer& vkb, const Mesh& mesh) const {
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

void RenderPipeline::renderMeshInstanced(VulkanCommandBuffer& vkb, const Mesh& mesh, const VulkanBuffer& vbo,
                                         const uint32_t count) const {
    std::array<VulkanVertexBufferBindRef, 2> vboBindings{};

    vboBindings[0] = {&mesh.vbo, 0};
    vboBindings[1] = {&vbo, 0};
    vkb.bindBuffers(vboBindings);

    if (mesh.ibo) {
        vkb.bindIndexBuffer(mesh.ibo, 0, mesh.indexType);
    }

    vkb.drawIndexed(mesh.count, count, 0, 0, 0);
}
