#pragma once

#include "../font/font_face.hpp"
#include "../math/matrix.hpp"
#include "../vulkan/vulkan_renderer.hpp"

namespace Engine {
class Canvas {
public:
    explicit Canvas(VulkanRenderer& vulkan);

    void begin(const Vector2i& viewport);
    void end(VulkanCommandBuffer& vkb);
    void scissor(const Vector2& pos, const Vector2& size);
    void rect(const Vector2& pos, const Vector2& size, const Color4& color);
    void rectOutline(const Vector2& pos, const Vector2& size, const Color4& color);
    void text(const Vector2& pos, const std::string& text, const FontFace& font, float height, const Color4& color);
    void image(const Vector2& pos, const Vector2& size, const VulkanTexture& texture, const Color4& color);

private:
    struct Vertex {
        Vector2 pos;
        Vector2 uv;
        Vector4 color;
    };

    struct CommandDraw {
        size_t start{0};
        size_t length{0};
        const VulkanTexture* texture{nullptr};
        int mode{0};
    };

    struct CommandScissor {
        Vector2i pos;
        Vector2i size;
    };

    struct Command {
        union {
            CommandDraw draw{};
            CommandScissor scissor;
        };

        enum Type {
            None,
            Draw,
            Scissor,
        } type = Type::None;

        Command() = default;
    };

    struct UniformBuffer {
        Matrix4 mvp;
    };

    void createPipeline();
    void createDescriptorSetLayout();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();
    void createDefaultTexture();
    CommandDraw& addDrawCommand();
    Vertex* allocate();

    VulkanRenderer& vulkan;
    Vector2i lastViewport;
    VulkanPipeline pipeline;
    VulkanDescriptorSetLayout descriptorSetLayout;
    VulkanDoubleBuffer vbo;
    VulkanDoubleBuffer ibo;
    VulkanDoubleBuffer ubo;
    VulkanTexture defaultTexture;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<Command> commands;
    size_t vertexOffset{0};
    size_t commandCount{0};
    size_t indexOffset{0};
};
} // namespace Engine
