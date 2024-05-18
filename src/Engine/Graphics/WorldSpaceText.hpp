#pragma once

#include "../Font/TextShaper.hpp"
#include "Mesh.hpp"

namespace Engine {
class WorldSpaceText {
public:
    struct Vertex {
        Vector3 position;
        Vector2 offset;
        Vector2 size;
        Vector2 uv;
        Vector2 st;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                {1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, offset)},
                {2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, size)},
                {3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)},
                {4, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, st)},
            };
        };
    };

    using Vertices = std::vector<Vertex>;

    class TextVertexShaper : public TextShaper {
    public:
        TextVertexShaper(Vertices& vertices, const FontFamily& font, float size, const Vector3& pos);

    private:
        void onGlyph(const FontFace& fontFace, const FontFace::Glyph& glyph, const Vector2& pen, const Quad& quad,
                     const Color4& color) override;

        Vertices& vertices;
        Vector3 pos;
    };

    explicit WorldSpaceText(const FontFamily& font);

    void add(const Vector3& pos, const std::string_view& text, float size);
    void recalculate(VulkanRenderer& vulkan);
    void clear();

    const Mesh& getMesh() const {
        return mesh;
    }

    const FontFamily& getFont() const {
        return font;
    }

private:
    const FontFamily& font;
    Vertices vertices;
    Mesh mesh;
    TextAlign textAlign{TextAlign::Center};
};
} // namespace Engine
