#include "RendererCanvas.hpp"
#include <canvas_frag.spirv.h>
#include <canvas_vert.spirv.h>

using namespace Engine;

RendererCanvas::RendererCanvas(VulkanRenderer& vulkan) : vulkan{vulkan} {
    try {
        create();
        createDefaultTexture();
    } catch (...) {
        EXCEPTION_NESTED("Failed to create canvas renderer");
    }
}

void RendererCanvas::reset() {
    auto& descriptorPool = descriptorPools.at(vulkan.getCurrentFrameNum());
    descriptorPool.reset();
}

void RendererCanvas::render(VulkanCommandBuffer& vkb, Canvas& canvas, const Vector2i& viewport) {
    if (!canvas.hasData()) {
        return;
    }

    vkb.bindPipeline(pipeline);

    std::array<VulkanVertexBufferBindRef, 1> vbos{};
    vbos[0] = {&canvas.getVbo(), 0};
    vkb.bindBuffers(vbos);
    vkb.bindIndexBuffer(canvas.getIbo(), 0, VK_INDEX_TYPE_UINT32);

    const auto mvp =
        glm::ortho(0.0f, static_cast<float>(viewport.x), 0.0f, static_cast<float>(viewport.y), -1.0f, 1.0f);

    constexpr auto flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    vkb.pushConstants(pipeline, flags, 0, sizeof(Matrix4), &mvp);

    const auto& textures = canvas.getSamplerArray();

    auto& descriptorPool = descriptorPools.at(vulkan.getCurrentFrameNum());
    auto descriptorSet = descriptorPool.createDescriptorSet(descriptorSetLayout);

    std::array<VkDescriptorImageInfo, 16> imageInfo{};

    for (size_t i = 0; i < textures.size(); i++) {
        imageInfo[i] = VkDescriptorImageInfo{};
        imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (textures.at(i)) {
            imageInfo[i].imageView = textures.at(i)->getImageView();
            imageInfo[i].sampler = textures.at(i)->getSampler();
        } else {
            imageInfo[i].imageView = defaultTexture.getImageView();
            imageInfo[i].sampler = defaultTexture.getSampler();
        }
    }

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet.getHandle();
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 16;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pImageInfo = imageInfo.data();
    descriptorWrite.pTexelBufferView = nullptr;
    descriptorWrite.pNext = nullptr;

    descriptorSet.bind({&descriptorWrite, 1});
    vkb.bindDescriptorSet(descriptorSet, pipeline.getLayout(), pipeline.isCompute());

    for (const auto& batch : canvas.getBatches()) {
        if (!batch.length) {
            continue;
        }

        // vkb.setScissor(batch.scissor.pos, batch.scissor.size);
        vkb.drawIndexedIndirect(canvas.getCbo(),
                                batch.offset * sizeof(VkDrawIndexedIndirectCommand),
                                batch.length,
                                sizeof(VkDrawIndexedIndirectCommand));
    }
}

void RendererCanvas::create() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 16;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    descriptorSetLayout = vulkan.createDescriptorSetLayout({samplerLayoutBinding});

    // Descriptor pool
    VulkanDescriptorPool::CreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    static constexpr size_t maxSetsPerPool = 128;

    VkDescriptorPoolSize descriptorPoolSize;
    descriptorPoolSize.type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSize.descriptorCount = 1 * maxSetsPerPool;

    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &descriptorPoolSize;
    poolInfo.maxSets = maxSetsPerPool;

    for (auto& descriptorPool : descriptorPools) {
        descriptorPool = vulkan.createDescriptorPool(poolInfo);
    }

    // Shader
    shaderVert = vulkan.createShaderModule(Embed::canvas_vert_spirv, VK_SHADER_STAGE_VERTEX_BIT);
    shaderFrag = vulkan.createShaderModule(Embed::canvas_frag_spirv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&shaderVert, &shaderFrag};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Canvas::Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Canvas::Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Canvas::Vertex, uv);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Canvas::Vertex, color);

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
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
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
    pushConstantRange.size = sizeof(Matrix4);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineInfo.pipelineLayoutInfo.setLayoutCount = 1;
    pipelineInfo.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.getHandle();
    pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineInfo.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    pipeline = vulkan.createPipeline(pipelineInfo);
}

void RendererCanvas::createDefaultTexture() {
    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {4, 4, 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    defaultTexture = vulkan.createTexture(textureInfo);

    auto pixels = std::unique_ptr<char[]>(new char[4 * 4 * 4]);
    std::memset(pixels.get(), 0xFF, 4 * 4 * 4);

    vulkan.transitionImageLayout(defaultTexture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.copyDataToImage(defaultTexture, 0, {0, 0}, 0, {4, 4}, pixels.get());
    vulkan.transitionImageLayout(
        defaultTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
