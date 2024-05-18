#include "TextShaper.hpp"
#include "FontFamily.hpp"
#include <utf8.h>

using namespace Engine;

enum Modifiers : uint32_t {
    IsRegular = 0x01,
    IsBold = 0x02,
    IsLight = 0x04,
    IsItalic = 0x08,
    IsPrimaryColor = 0x10,
    IsSecondaryColor = 0x20,
    IsTernaryColor = 0x40,
    IsGrayColor = 0x80,
};

static uint32_t codeToModifier(const uint32_t code) {
    switch (code) {
    case 'r':
        return IsRegular;
    case 'b':
        return IsBold;
    case 'l':
        return IsLight;
    case 'i':
        return IsItalic;
    case 'p':
        return IsPrimaryColor;
    case 's':
        return IsSecondaryColor;
    case 't':
        return IsTernaryColor;
    case 'g':
        return IsGrayColor;
    default:
        return 0;
    }
}

const FontFace* getFontFace(const FontFamily& font, const uint32_t modifiers) {
    if (modifiers & IsLight) {
        if (modifiers & IsItalic) {
            return &font.get(FontType::LightItalic);
        } else {
            return &font.get(FontType::Light);
        }
    } else if (modifiers & IsBold) {
        if (modifiers & IsItalic) {
            return &font.get(FontType::BoldItalic);
        } else {
            return &font.get(FontType::Bold);
        }
    } else {
        if (modifiers & IsItalic) {
            return &font.get(FontType::RegularItalic);
        } else {
            return &font.get(FontType::Regular);
        }
    }
}

static Color4 getFontColor(const Color4& mainColor, const uint32_t modifiers) {
    if (modifiers & IsPrimaryColor) {
        return mainColor * Colors::primary;
    } else if (modifiers & IsSecondaryColor) {
        return mainColor * Colors::success;
    } else if (modifiers & IsTernaryColor) {
        return mainColor * Colors::danger;
    } else if (modifiers & IsGrayColor) {
        return mainColor * Colors::textGray;
    } else {
        return mainColor;
    }
}

TextShaper::TextShaper(const FontFamily& font, const float size, const Vector2& pos, const Color4& color) :
    font{font},
    size{size},
    origin{pos},
    pos{pos},
    bounds{pos},
    scale{static_cast<float>(size) / static_cast<float>(font.getSize())},
    mainColor{color} {
}

void TextShaper::write(const std::string_view& text) {
    // const auto total = utf8::distance(text.begin(), text.end());
    auto it = text.begin();
    unsigned int previous = 0;
    const char* cmdStart = nullptr;
    std::string_view cmd;

    uint32_t modifiers{IsRegular};
    const auto* face = getFontFace(font, modifiers);
    auto color{mainColor};

    while (it < text.end()) {
        const auto code = utf8::next(it, text.end());

        // Is it a beginning of a command?
        if (code == '<' && previous != '\\') {
            cmdStart = it;
            continue;
        }
        // Is it an end of the command?
        else if (cmdStart && code == '>') {
            cmd = std::string_view{cmdStart, static_cast<size_t>(it - cmdStart - 1)};
            cmdStart = nullptr;

            if (cmd.size() == 1) {
                modifiers |= codeToModifier(cmd[0]);
            } else if (cmd.size() == 2 && cmd[0] == '/') {
                modifiers &= ~codeToModifier(cmd[1]);
            }

            face = getFontFace(font, modifiers);
            color = getFontColor(mainColor, modifiers);
            continue;
        }
        // Are we inside a command?
        else if (cmdStart) {
            continue;
        }

        previous = code;

        const auto& glyph = face->getGlyph(code);

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

        onGlyph(*face, glyph, pos, quad, color);

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
                         const TextShaper::Quad& quad, const Color4& color) {
    (void)fontFace;
    (void)glyph;
    (void)pen;
    (void)quad;
    (void)color;
}
