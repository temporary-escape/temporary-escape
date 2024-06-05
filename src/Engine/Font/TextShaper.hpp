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

    void resetPen(const Vector2& value = {0.0f, 0.0f});

private:
    virtual void onGlyph(const FontFace& fontFace, const FontFace::Glyph& glyph, const Vector2& pen, const Quad& quad,
                         const Color4& color, const char* it, uint32_t code);

    const FontFamily& font;
    float size;
    Vector2 origin;
    Vector2 pos;
    Vector2 bounds;
    float scale;
    Color4 mainColor;
};

class TextWrapper : public TextShaper {
public:
    TextWrapper(const FontFamily& font, float size, float maxWidth);

    [[nodiscard]] const std::string& getResult() const {
        return result;
    }
    void flush(std::string_view::const_iterator end);
    float getHeight() const;

private:
    void onGlyph(const FontFace& fontFace, const FontFace::Glyph& glyph, const Vector2& pen, const Quad& quad,
                 const Color4& color, const char* it, uint32_t code) override;

    float size;
    float maxWidth;
    const char* startChar{nullptr};
    const char* lastWordChar{nullptr};
    const char* previousChar{nullptr};
    std::string result;
    int numLines{0};
};
} // namespace Engine
