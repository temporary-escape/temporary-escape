#pragma once

#include "../Font/FontFace.hpp"
#include "../Vulkan/VulkanDevice.hpp"

namespace Engine {
class Canvas {
public:
    explicit Canvas(VulkanDevice& vulkan);

    void begin(const Vector2i& viewport);
    void end();
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
        Vector4 mode;
    };

    struct Command {
        union {
            struct {
                size_t start{0};
                size_t length{0};
                VkPrimitiveTopology primitive{VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
                const VulkanTexture* texture{nullptr};
            } draw{};

            struct {
                Vector2i pos;
                Vector2i size;
            } scissor;
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

    VulkanDevice& vulkan;
    Vector2i lastViewport;
    VulkanPipeline shader;
    VulkanBuffer vbo;
    VulkanVertexInputFormat vboFormat;
    VulkanBuffer ubo;
    VulkanTexture defaultTexture;
    std::vector<Vertex> vertices;
    std::vector<Command> commands;
};
} // namespace Engine
