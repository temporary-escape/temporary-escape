#include "canvas.hpp"
#include "../utils/log.hpp"
#include <utf8cpp/utf8.h>

#define CMP "VulkanCanvas"

using namespace Engine;

static const std::string vertexShaderSource = R"(#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 mvp;
} uniforms;

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoords;
layout(location = 2) in vec4 in_Color;

layout(location = 0) out VS_OUT {
    vec4 color;
    vec2 texCoords;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vs_out.color = in_Color;
    vs_out.texCoords = in_TexCoords;
    gl_Position = uniforms.mvp * vec4(in_Position, 0.0, 1.0);
}
)";

static const std::string fragmentShaderSource = R"(#version 450

layout(location = 0) in VS_OUT {
    vec4 color;
    vec2 texCoords;
} ps_in;

layout(push_constant) uniform Constants {
	int mode;
} constants;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 raw = texture(texSampler, ps_in.texCoords);
    if (constants.mode == 1) {
        outColor = ps_in.color * vec4(1.0, 1.0, 1.0, raw.r);
    } else {
        outColor = raw * ps_in.color;
    }
}
)";

Canvas::Canvas(VulkanRenderer& vulkan) : vulkan{vulkan} {
    vertices.resize(48 * 1024);
    indices.resize(64 * 1024);
    commands.resize(1024);

    createDescriptorSetLayout();
    createPipeline();
    createUniformBuffer();
    createIndexBuffer();
    createVertexBuffer();
    createDefaultTexture();
}

void Canvas::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    descriptorSetLayout = vulkan.createDescriptorSetLayout({uboLayoutBinding, samplerLayoutBinding});
}

void Canvas::createPipeline() {
    auto vert = vulkan.createShaderModule(vertexShaderSource, VK_SHADER_STAGE_VERTEX_BIT);
    auto frag = vulkan.createShaderModule(fragmentShaderSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VulkanPipeline::CreateInfo pipelineInfo{};
    pipelineInfo.shaderModules = {&vert, &frag};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, uv);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, color);

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

    pipeline = vulkan.createPipeline(pipelineInfo);
}

void Canvas::createVertexBuffer() {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vbo = vulkan.createDoubleBuffer(bufferInfo);
}

void Canvas::createIndexBuffer() {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ibo = vulkan.createDoubleBuffer(bufferInfo);
}

void Canvas::createUniformBuffer() {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(UniformBuffer);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ubo = vulkan.createDoubleBuffer(bufferInfo);
}

void Canvas::createDefaultTexture() {
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
    vulkan.transitionImageLayout(defaultTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Canvas::begin(const Vector2i& viewport) {
    lastViewport = viewport;
    vertexOffset = 0;
    commandCount = 0;
    indexOffset = 0;

    UniformBuffer uniformBuffer{};
    uniformBuffer.mvp =
        glm::ortho(0.0f, static_cast<float>(viewport.x), 0.0f, static_cast<float>(viewport.y), -1.0f, 1.0f);

    auto* uboData = ubo.mapMemory();
    std::memcpy(uboData, &uniformBuffer, sizeof(uniformBuffer));
    ubo.unmapMemory();
}

void Canvas::end(VulkanCommandBuffer& vkb) {
    if (commandCount == 0) {
        return;
    }

    /*if (vertices.capacity() != vbo.getSize()) {
        createVertexBuffer();
    }*/

    auto* vboData = vbo.mapMemory();
    std::memcpy(vboData, vertices.data(), vertices.size() * sizeof(Vertex));
    vbo.unmapMemory();

    auto* iboData = ibo.mapMemory();
    std::memcpy(iboData, indices.data(), indices.size() * sizeof(uint16_t));
    ibo.unmapMemory();

    vkb.bindPipeline(pipeline);
    vkb.setViewport({0, 0}, lastViewport);
    vkb.setScissor({0, 0}, lastViewport);
    vkb.bindBuffers({{vbo.getCurrentBuffer(), 0}});
    vkb.bindIndexBuffer(ibo.getCurrentBuffer(), 0, VK_INDEX_TYPE_UINT16);

    /*for (size_t i = 0; i < commandCount; i++) {
        const auto& cmd = commands.at(i);

        // Simple draw command
        if (cmd.type == Command::Type::Draw) {
            vkb.pushConstants(pipeline, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int32_t), &cmd.draw.mode);

            if (cmd.draw.texture) {
                vkb.bindDescriptors(pipeline, descriptorSetLayout, {{0, &ubo.getCurrentBuffer()}},
                                    {{1, cmd.draw.texture}});
            } else {
                vkb.bindDescriptors(pipeline, descriptorSetLayout, {{0, &ubo.getCurrentBuffer()}},
                                    {{1, &defaultTexture}});
            }
            vkb.drawIndexed(cmd.draw.length, 1, cmd.draw.start, 0, 0);
        }
        // Scissor command
        else if (cmd.type == Command::Type::Scissor) {
            vkb.setScissor(cmd.scissor.pos, cmd.scissor.size);
        }
    }*/

    const auto flush = [&](Command& cmd) {
        // Simple draw command
        if (cmd.type == Command::Type::Draw) {
            vkb.pushConstants(pipeline, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int32_t), &cmd.draw.mode);

            if (cmd.draw.texture) {
                vkb.bindDescriptors(pipeline, descriptorSetLayout, {{0, &ubo.getCurrentBuffer()}},
                                    {{1, cmd.draw.texture}});
            } else {
                vkb.bindDescriptors(pipeline, descriptorSetLayout, {{0, &ubo.getCurrentBuffer()}},
                                    {{1, &defaultTexture}});
            }
            vkb.drawIndexed(cmd.draw.length, 1, cmd.draw.start, 0, 0);
        }
        // Scissor command
        else if (cmd.type == Command::Type::Scissor) {
            vkb.setScissor(cmd.scissor.pos, cmd.scissor.size);
        }
    };

    Command* previous = &commands.at(0);

    for (size_t i = 1; i < commandCount; i++) {
        auto& next = commands.at(i);
        if (previous->canMerge(next)) {
            previous->merge(next);
        } else {
            flush(*previous);
            previous = &next;
        }
    }

    if (previous) {
        flush(*previous);
    }

    vkb.setScissor({0, 0}, lastViewport);
}

Canvas::Command& Canvas::addCommand() {
    if (commandCount + 1 >= commands.size()) {
        commands.resize(commands.size() + 1024);
    }

    auto& cmd = commands.at(commandCount++);

    return cmd;
}

Canvas::CommandDraw& Canvas::addDrawCommand() {
    auto& cmd = addCommand();
    cmd.type = Command::Type::Draw;
    return cmd.draw;
}

Canvas::CommandScissor& Canvas::addScissorCommand() {
    auto& cmd = addCommand();
    cmd.type = Command::Type::Scissor;
    return cmd.scissor;
}

Canvas::Vertex* Canvas::allocate() {
    const auto start = vertexOffset;

    if (vertexOffset + 4 >= vertices.size()) {
        vertices.resize(vertices.size() + 1024 * 4);
    }

    if (indexOffset + 6 >= indices.size()) {
        indices.resize(indices.size() + 1024 * 6);
    }

    indices[indexOffset + 0] = vertexOffset;
    indices[indexOffset + 1] = vertexOffset + 1;
    indices[indexOffset + 2] = vertexOffset + 2;
    indices[indexOffset + 3] = vertexOffset + 2;
    indices[indexOffset + 4] = vertexOffset + 3;
    indices[indexOffset + 5] = vertexOffset;

    vertexOffset += 4;
    indexOffset += 6;

    return &vertices.at(start);
}

void Canvas::scissor(const Vector2& pos, const Vector2& size) {
    auto& cmd = addScissorCommand();
    cmd.pos = pos;
    cmd.size = size;
}

void Canvas::rect(const Vector2& pos, const Vector2& size) {
    auto& cmd = addDrawCommand();
    cmd.start = indexOffset;
    cmd.length = 6;

    const auto dst = allocate();

    dst[0].pos = pos;
    dst[1].pos = pos + Vector2{size.x, 0.0f};
    dst[2].pos = pos + Vector2{size.x, size.y};
    dst[3].pos = pos + Vector2{0.0f, size.y};

    dst[0].color = nextColor;
    dst[1].color = nextColor;
    dst[2].color = nextColor;
    dst[3].color = nextColor;
}

void Canvas::rectOutline(const Vector2& pos, const Vector2& size, const float thickness) {
    rect(Vector2{pos.x, pos.y}, Vector2{size.x, thickness});
    rect(Vector2{pos.x, pos.y + thickness}, Vector2{thickness, size.y - thickness * 2.0f});
    rect(Vector2{pos.x + size.x - thickness, pos.y + thickness}, Vector2{thickness, size.y - thickness * 2.0f});
    rect(Vector2{pos.x, pos.y + size.y - thickness}, Vector2{size.x, thickness});

    /*const auto start = vertices.size();

    auto& v0 = vertices.emplace_back();
    auto& v1 = vertices.emplace_back();
    auto& v2 = vertices.emplace_back();
    auto& v3 = vertices.emplace_back();
    auto& v4 = vertices.emplace_back();

    v0.pos = pos;
    v1.pos = pos + Vector2{0.0f, size.y};
    v2.pos = pos + size;
    v3.pos = pos + Vector2{size.x, 0.0f};

    v0.color = color;
    v1.color = color;
    v2.color = color;
    v3.color = color;

    v0.alpha = 0.0f;
    v1.alpha = 0.0f;
    v2.alpha = 0.0f;
    v3.alpha = 0.0f;

    v4 = v0;

    auto& cmd = commands.emplace_back();
    cmd.type = Command::Type::Draw;
    cmd.draw.start = start;
    cmd.draw.length = 5;
    cmd.draw.primitive = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;*/
}

void Canvas::text(const Vector2& pos, const std::string& text, const FontFace& font, const float height) {

    size_t start = indexOffset;

    auto it = text.c_str();
    const auto end = it + text.size();

    size_t total = 0;
    auto pen = pos;

    const auto scale = height / font.getSize();

    while (it < end) {
        const auto code = utf8::next(it, end);
        const auto& glyph = font.getGlyph(code);
        total++;

        const auto p = pen + Vector2{0.0f, glyph.ascend * scale};

        auto* dst = allocate();

        dst[0].pos = p + Vector2{0.0f, -glyph.size.y * scale};
        dst[0].color = nextColor;
        dst[0].uv = Vector2{glyph.uv.x, glyph.uv.y};

        dst[1].pos = p + Vector2{glyph.size.x * scale, -glyph.size.y * scale};
        dst[1].color = nextColor;
        dst[1].uv = Vector2{glyph.uv.x + glyph.st.x, glyph.uv.y};

        dst[2].pos = p + Vector2{glyph.size.x * scale, 0.0f};
        dst[2].color = nextColor;
        dst[2].uv = Vector2{glyph.uv.x + glyph.st.x, glyph.uv.y + glyph.st.y};

        dst[3].pos = p;
        dst[3].color = nextColor;
        dst[3].uv = Vector2{glyph.uv.x, glyph.uv.y + glyph.st.y};

        pen += Vector2{glyph.advance * scale, 0.0f};
    }

    auto& cmd = addDrawCommand();
    cmd.mode = 1;
    cmd.start = start;
    cmd.length = total * 6;
    cmd.texture = &font.getTexture();
}

void Canvas::image(const Vector2& pos, const Vector2& size, const VulkanTexture& texture) {
    /*const auto start = vertices.size();

    auto& v0 = vertices.emplace_back();
    auto& v1 = vertices.emplace_back();
    auto& v2 = vertices.emplace_back();

    auto& v3 = vertices.emplace_back();
    auto& v4 = vertices.emplace_back();
    auto& v5 = vertices.emplace_back();

    v0.pos = pos;
    v0.uv = {0.0f, 1.0f};
    v0.alpha = 1.0f;
    v1.pos = pos + Vector2{0.0f, size.y};
    v1.uv = {0.0f, 0.0f};
    v1.alpha = 1.0f;
    v2.pos = pos + Vector2{size.x, 0.0f};
    v2.uv = {1.0f, 1.0f};
    v2.alpha = 1.0f;

    v0.color = color;
    v1.color = color;
    v2.color = color;

    v3 = v2;
    v4 = v1;
    v5.pos = pos + size;
    v5.color = color;
    v5.uv = {1.0f, 0.0f};
    v5.alpha = 1.0f;

    auto& cmd = commands.emplace_back();
    cmd.type = Command::Type::Draw;
    cmd.draw.start = start;
    cmd.draw.length = 6;
    cmd.draw.texture = &texture;*/
}
