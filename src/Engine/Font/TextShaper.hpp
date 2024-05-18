#pragma once

#include "FontFace.hpp"

namespace Engine {
class ENGINE_API FontFamily;

class ENGINE_API TextShaper {
public:
    struct Vertex {
        Vector2 pos;
        Vector2 uv;
    };

    struct Quad {
        Vertex vertices[4];
        float advance;
        float ascend;
    };

    TextShaper(const FontFamily& font, float size, const Vector2& pos = {0.0f, 0.0f},
               const Color4& color = {1.0f, 1.0f, 1.0f, 1.0f});

    virtual void write(const std::string_view& text);

    float getScale() const {
        return scale;
    }

    Vector2 getBounds() const;

private:
    virtual void onGlyph(const FontFace& fontFace, const FontFace::Glyph& glyph, const Vector2& pen, const Quad& quad,
                         const Color4& color);

    const FontFamily& font;
    float size;
    Vector2 origin;
    Vector2 pos;
    Vector2 bounds;
    float scale;
    Color4 mainColor;
};
} // namespace Engine
