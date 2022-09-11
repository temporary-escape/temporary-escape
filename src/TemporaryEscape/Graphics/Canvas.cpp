#include "Canvas.hpp"
#include "../Utils/Log.hpp"
#include <utf8cpp/utf8.h>

#define CMP "VulkanCanvas"

using namespace Engine;

static const std::string vertexShaderSource = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 mvp;
};

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoords;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec4 in_Mode;

layout(location = 0) out VS_OUT
{
    vec4 color;
    vec2 texCoords;
    float alphaSource;
} vs_out;

out gl_PerVertex{
    vec4 gl_Position;
};

void main()
{
    vs_out.color = in_Color;
    vs_out.texCoords = in_TexCoords;
    vs_out.alphaSource = in_Mode.x;
    gl_Position = mvp * vec4(in_Position, 0.0f, 1.0f);
}
)";

static const std::string fragmentShaderSource = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT
{
    vec4 color;
    vec2 texCoords;
    float alphaSource;
} ps_in;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 raw = texture(texSampler, ps_in.texCoords);
    outColor = mix(raw * ps_in.color, ps_in.color * raw.r, 1.0);
}
)";

Canvas::Canvas(VulkanDevice& vulkan) : vulkan{vulkan} {
    shader = vulkan.createPipeline({
        {"", fragmentShaderSource, ShaderType::Fragment},
        {"", vertexShaderSource, ShaderType::Vertex},
    });

    vertices.reserve(64 * 1024);
    commands.reserve(1024);

    vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic,
                              sizeof(Vertex) * vertices.capacity());
    ubo = vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic, sizeof(UniformBuffer));
    vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec2},
                {1, 0, VulkanVertexInputFormat::Format::Vec2},
                {2, 0, VulkanVertexInputFormat::Format::Vec4},
                {3, 0, VulkanVertexInputFormat::Format::Vec4},
            },
        },
    });

    VulkanTexture::Descriptor textureDesc{};
    textureDesc.format = VulkanTexture::Format::VK_FORMAT_R8G8B8A8_UNORM;
    textureDesc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    textureDesc.size = {4, 4};
    defaultTexture = vulkan.createTexture(textureDesc);

    auto pixels = std::unique_ptr<char[]>(new char[4 * 4 * 4]);
    std::memset(pixels.get(), 0xFF, 4 * 4 * 4);
    defaultTexture.subData(0, {0, 0}, {4, 4}, pixels.get());
}

void Canvas::begin(const Vector2i& viewport) {
    lastViewport = viewport;
    vertices.clear();
    commands.clear();

    auto* data = ubo.map<UniformBuffer>();
    data->mvp = glm::ortho(0.0f, static_cast<float>(viewport.x), 0.0f, static_cast<float>(viewport.y), -1.0f, 1.0f);
    ubo.unmap();
}

void Canvas::end() {
    if (commands.empty()) {
        return;
    }

    auto data = vbo.mapPtr(sizeof(Vertex) * vertices.capacity());
    std::memcpy(data, vertices.data(), sizeof(Vertex) * vertices.capacity());
    vbo.unmap();

    vulkan.bindPipeline(shader);
    vulkan.bindUniformBuffer(ubo, 0);
    vulkan.setDepthStencilState(false, false);
    VulkanBlendState blendState{};
    blendState.blendEnable = true;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});
    vulkan.bindVertexBuffer(vbo, 0);
    vulkan.bindVertexInputFormat(vboFormat);

    for (const auto& cmd : commands) {
        if (cmd.type == Command::Type::Draw) {
            vulkan.setInputAssembly(cmd.draw.primitive);
            if (cmd.draw.texture) {
                vulkan.bindTexture(*cmd.draw.texture, 1);
            } else {
                vulkan.bindTexture(defaultTexture, 1);
            }
            vulkan.draw(cmd.draw.length, 1, cmd.draw.start, 0);
        } else if (cmd.type == Command::Type::Scissor) {
            vulkan.setScissor(cmd.scissor.pos, cmd.scissor.size);
        }
    }

    vulkan.setScissor({0, 0}, lastViewport);
}

void Canvas::scissor(const Vector2& pos, const Vector2& size) {
    auto& cmd = commands.emplace_back();
    cmd.type = Command::Type::Scissor;
    cmd.scissor.pos = pos;
    cmd.scissor.size = size;
}

void Canvas::rect(const Vector2& pos, const Vector2& size, const Color4& color) {
    const auto start = vertices.size();

    auto& v0 = vertices.emplace_back();
    auto& v1 = vertices.emplace_back();
    auto& v2 = vertices.emplace_back();

    auto& v3 = vertices.emplace_back();
    auto& v4 = vertices.emplace_back();
    auto& v5 = vertices.emplace_back();

    v0.pos = pos;
    v1.pos = pos + Vector2{0.0f, size.y};
    v2.pos = pos + Vector2{size.x, 0.0f};

    v0.color = color;
    v1.color = color;
    v2.color = color;

    v0.mode.x = 0.0f;
    v1.mode.x = 0.0f;
    v2.mode.x = 0.0f;

    v3 = v2;
    v4 = v1;
    v5.pos = pos + size;
    v5.color = color;
    v5.mode.x = 0.0f;

    auto& cmd = commands.emplace_back();
    cmd.type = Command::Type::Draw;
    cmd.draw.start = start;
    cmd.draw.length = 6;
    cmd.draw.primitive = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void Canvas::rectOutline(const Vector2& pos, const Vector2& size, const Color4& color) {
    const auto start = vertices.size();

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

    v0.mode.x = 0.0f;
    v1.mode.x = 0.0f;
    v2.mode.x = 0.0f;
    v3.mode.x = 0.0f;

    v4 = v0;

    auto& cmd = commands.emplace_back();
    cmd.type = Command::Type::Draw;
    cmd.draw.start = start;
    cmd.draw.length = 5;
    cmd.draw.primitive = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
}

void Canvas::text(const Vector2& pos, const std::string& text, const FontFace& font, const float height,
                  const Color4& color) {
    const auto start = vertices.size();

    auto it = text.c_str();
    const auto end = it + text.size();

    size_t total = 0;
    auto pen = pos /* + Vector2{0.0f, font.getSize()}*/;

    const auto scale = height / font.getSize();

    while (it < end) {
        const auto code = utf8::next(it, end);
        const auto& glyph = font.getGlyph(code);
        total++;

        const auto p = pen + Vector2{0.0f, glyph.ascend * scale};

        auto& v0 = vertices.emplace_back();
        auto& v1 = vertices.emplace_back();
        auto& v2 = vertices.emplace_back();
        auto& v3 = vertices.emplace_back();
        auto& v4 = vertices.emplace_back();
        auto& v5 = vertices.emplace_back();

        v0.pos = p + Vector2{0.0f, -glyph.size.y * scale};
        v0.color = color;
        v0.uv = Vector2{glyph.uv.x, glyph.uv.y};
        v0.mode.x = 1.0f;

        v1.pos = p + Vector2{0.0f, 0.0f};
        v1.color = color;
        v1.uv = Vector2{glyph.uv.x, glyph.uv.y + glyph.st.y};
        v1.mode.x = 1.0f;

        v2.pos = p + Vector2{glyph.size.x * scale, -glyph.size.y * scale};
        v2.color = color;
        v2.uv = Vector2{glyph.uv.x + glyph.st.x, glyph.uv.y};
        v2.mode.x = 1.0f;

        v3 = v2;
        v4 = v1;

        v5.pos = p + Vector2{glyph.size.x * scale, 0.0f};
        v5.color = color;
        v5.uv = Vector2{glyph.uv.x + glyph.st.x, glyph.uv.y + glyph.st.y};
        v5.mode.x = 1.0f;

        pen += Vector2{glyph.advance * scale, 0.0f};
    }

    auto& cmd = commands.emplace_back();
    cmd.type = Command::Type::Draw;
    cmd.draw.start = start;
    cmd.draw.length = total * 6;
    cmd.draw.texture = &font.getTexture();
    cmd.draw.primitive = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void Canvas::image(const Vector2& pos, const Vector2& size, const VulkanTexture& texture, const Color4& color) {
    const auto start = vertices.size();

    auto& v0 = vertices.emplace_back();
    auto& v1 = vertices.emplace_back();
    auto& v2 = vertices.emplace_back();

    auto& v3 = vertices.emplace_back();
    auto& v4 = vertices.emplace_back();
    auto& v5 = vertices.emplace_back();

    v0.pos = pos;
    v0.uv = {0.0f, 1.0f};
    v0.mode.x = 1.0f;
    v1.pos = pos + Vector2{0.0f, size.y};
    v1.uv = {0.0f, 0.0f};
    v1.mode.x = 1.0f;
    v2.pos = pos + Vector2{size.x, 0.0f};
    v2.uv = {1.0f, 1.0f};
    v2.mode.x = 1.0f;

    v0.color = color;
    v1.color = color;
    v2.color = color;

    v3 = v2;
    v4 = v1;
    v5.pos = pos + size;
    v5.color = color;
    v5.uv = {1.0f, 0.0f};
    v5.mode.x = 1.0f;

    auto& cmd = commands.emplace_back();
    cmd.type = Command::Type::Draw;
    cmd.draw.start = start;
    cmd.draw.length = 6;
    cmd.draw.texture = &texture;
    cmd.draw.primitive = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}
