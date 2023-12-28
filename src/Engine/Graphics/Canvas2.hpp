#pragma once

#include "../Assets/Image.hpp"
#include "../Font/FontFamily.hpp"
#include "../Math/Matrix.hpp"
#include "../Vulkan/VulkanRenderer.hpp"

namespace Engine {
class ENGINE_API Canvas2 {
public:
    struct Vertex {
        Vector2 pos;
        Vector4 uv;
        Vector4 color;
    };

    struct Batch {
        struct {
            Vector2i pos;
            Vector2i size;
        } scissor;
        size_t offset;
        size_t length;
    };

    using SamplerArray = std::array<const VulkanTexture*, 16>;
    using Batches = std::vector<Batch>;

    explicit Canvas2(VulkanRenderer& vulkan);
    virtual ~Canvas2();

    void flush();
    void begin(const Vector2i& viewport);
    void setScissor(const Vector2& pos, const Vector2& size);
    void clearScissor();
    void drawRect(const Vector2& pos, const Vector2& size, const Color4& color);
    void drawRectOutline(const Vector2& pos, const Vector2& size, float thickness, const Color4& color);
    void drawTexture(const Vector2& pos, const Vector2& size, const VulkanTexture& texture, const Color4& color);
    void drawText(const Vector2& pos, const std::string_view& text, const FontFamily& font, int size,
                  const Color4& color);
    void drawImage(const Vector2& pos, const Vector2& size, const Image& image, const Color4& color);

    bool hasData() const {
        return vbo && ibo && cbo && commands.count > 0;
    }
    [[nodiscard]] const VulkanBuffer& getVbo() const {
        return vbo.getCurrentBuffer();
    }
    [[nodiscard]] const VulkanBuffer& getIbo() const {
        return ibo.getCurrentBuffer();
    }
    [[nodiscard]] const VulkanBuffer& getCbo() const {
        return cbo.getCurrentBuffer();
    }
    [[nodiscard]] const SamplerArray& getSamplerArray() const {
        return textures;
    }
    [[nodiscard]] const Batches& getBatches() const {
        return batches;
    }

private:
    template <typename T> struct Buffer {
        std::vector<T> data;
        size_t count{0};
    };

    void doRect(Canvas2::Vertex*& v, uint32_t*& i, const Vector2& pos, const Vector2& size, const Color4& color);

    template <typename T> VkBufferUsageFlags getBufferUsage() const;
    template <typename T> T* reserve(Buffer<T>& buffer, const size_t count);
    template <typename T> size_t getOffset(Buffer<T>& buffer, const T* ptr) const;
    template <typename T> void flush(Buffer<T>& buffer, VulkanDoubleBuffer& target);
    void resetTextures();
    size_t addTexture(const VulkanTexture& texture);

    VulkanRenderer& vulkan;

    VulkanDoubleBuffer vbo;
    VulkanDoubleBuffer ibo;
    VulkanDoubleBuffer cbo;

    Buffer<Vertex> vertices;
    Buffer<uint32_t> indices;
    Buffer<VkDrawIndexedIndirectCommand> commands;
    SamplerArray textures;
    Vector2i currentViewport;
    Batches batches;
};
} // namespace Engine
