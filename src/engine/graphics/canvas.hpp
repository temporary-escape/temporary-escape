#pragma once

#include "../assets/image.hpp"
#include "../font/font_face.hpp"
#include "../math/matrix.hpp"
#include "../vulkan/vulkan_renderer.hpp"

namespace Engine {
class ENGINE_API Canvas {
public:
    explicit Canvas(VulkanRenderer& vulkan);

    void begin(const Vector2i& viewport);
    void end(VulkanCommandBuffer& vkb);
    void scissor(const Vector2& pos, const Vector2& size);
    void color(const Color4& value) {
        nextColor = value;
    }
    void font(const FontFace& font, const int height) {
        currentFontFace = &font;
        currentFontHeight = height;
    }
    void rect(const Vector2& pos, const Vector2& size);
    void rectOutline(const Vector2& pos, const Vector2& size, float thickness);
    void text(const Vector2& pos, const std::string_view& text);
    void image(const Vector2& pos, const Vector2& size, const ImagePtr& asset) {
        image(pos, size, *asset);
    }
    void image(const Vector2& pos, const Vector2& size, const Image& asset);
    void image(const Vector2& pos, const Vector2& size, const VulkanTexture& texture, const Vector2& uv,
               const Vector2& st);

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

        [[nodiscard]] bool canMerge(const CommandDraw& other) const {
            return texture == other.texture && mode == other.mode;
        }

        void merge(const CommandDraw& other) {
            length += other.length;
        }
    };

    struct CommandScissor {
        Vector2i pos;
        Vector2i size;

        [[nodiscard]] bool canMerge(const CommandScissor& other) const {
            return false;
        }

        void merge(const CommandScissor& other) {
            (void)other;
        }
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

        [[nodiscard]] bool canMerge(const Command& other) const {
            if (type != other.type) {
                return false;
            }

            switch (type) {
            case Type::Draw: {
                return draw.canMerge(other.draw);
            }
            case Type::Scissor: {
                return scissor.canMerge(other.scissor);
            }
            default: {
                return false;
            }
            }
        }

        void merge(const Command& other) {
            switch (type) {
            case Type::Draw: {
                draw.merge(other.draw);
            }
            case Type::Scissor: {
                scissor.merge(other.scissor);
            }
            default: {
                break;
            }
            }
        }
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
    Command& addCommand();
    CommandDraw& addDrawCommand();
    CommandScissor& addScissorCommand();
    Vertex* allocate();

    VulkanRenderer& vulkan;
    const FontFace* currentFontFace{nullptr};
    int currentFontHeight{18};
    Vector2i lastViewport;
    VulkanPipeline pipeline;
    VulkanDescriptorSetLayout descriptorSetLayout;
    VulkanDoubleBuffer vbo;
    VulkanDoubleBuffer ibo;
    VulkanDoubleBuffer ubo;
    VulkanTexture defaultTexture;
    Color4 nextColor{1.0f};
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<Command> commands;
    size_t vertexOffset{0};
    size_t commandCount{0};
    size_t indexOffset{0};
};
} // namespace Engine
