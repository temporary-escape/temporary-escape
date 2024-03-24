#include "TextShaper.hpp"
#include "FontFamily.hpp"
#include <utf8.h>

using namespace Engine;

TextShaper::TextShaper(const FontFamily& font, const float size, const Vector2& pos) :
    font{font},
    size{size},
    origin{pos},
    pos{pos},
    bounds{pos},
    scale{static_cast<float>(size) / static_cast<float>(font.getSize())} {
}

void TextShaper::write(const std::string_view& text) {
    // const auto total = utf8::distance(text.begin(), text.end());
    const auto& face = font.get(FontFace::Regular);

    auto it = text.begin();
    while (it < text.end()) {
        const auto code = utf8::next(it, text.end());
        const auto& glyph = face.getGlyph(code);

        const auto p = pos + Vector2{0.0f, glyph.ascend * scale};

        Quad quad{
            {
                Vertex{
                    p + Vector2{0.0f, -glyph.size.y * scale},
                    Vector2{glyph.uv.x, glyph.uv.y},
                },
                Vertex{
                    p + Vector2{glyph.size.x * scale, -glyph.size.y * scale},
                    Vector2{glyph.uv.x + glyph.st.x, glyph.uv.y},
                },
                Vertex{
                    p + Vector2{glyph.size.x * scale, 0.0f},
                    Vector2{glyph.uv.x + glyph.st.x, glyph.uv.y + glyph.st.y},
                },
                Vertex{
                    p,
                    Vector2{glyph.uv.x, glyph.uv.y + glyph.st.y},
                },
            },
            glyph.advance * scale,
            glyph.ascend * scale,
        };

        bounds.x = glm::max(bounds.x, p.x + glyph.size.x * scale);
        bounds.y = glm::max(bounds.y, p.y + size);

        onGlyph(face, glyph, pos, quad);

        if (code == '\n') {
            pos.x = origin.x;
            pos.y += size * 1.25f;
        }

        pos += Vector2{quad.advance, 0.0f};
    }
}

Vector2 TextShaper::getBounds() const {
    return bounds - origin;
}

void TextShaper::onGlyph(const FontFace& fontFace, const FontFace::Glyph& glyph, const Vector2& pen,
                         const TextShaper::Quad& quad) {
    (void)fontFace;
    (void)glyph;
    (void)pen;
    (void)quad;
}
