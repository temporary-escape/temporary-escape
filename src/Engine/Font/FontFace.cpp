#include "FontFace.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Packer.hpp"
#include "FontLoader.hpp"
#include <utf8.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

FontFace::FontFace(VulkanRenderer& vulkan, const Span<uint8_t>& source, VulkanTexture& texture, const Vector2i& offset,
                   const int size) :
    size{size} {

    auto loader = FontLoader{source, size};

    std::vector<std::tuple<int, FontLoader::Glyph>> raw;

    static std::vector<std::tuple<int, int>> ranges = {
        {0x0000, 0x0000}, // Unknown,
        {0x0020, 0x00FF}, // Basic Latin + Latin-1 Supplement
        {0x0100, 0x017F}, // Latin Extended-A
        {0x0180, 0x024F}, // Latin Extended-B
        {0x0250, 0x02AF}, // IPA Extensions
        {0x0400, 0x04FF}, // Cyrillic
        {0x0500, 0x052F}, // Cyrillic Supplementary
        {0x2200, 0x22FF}, // Mathematical Operators
    };

    for (const auto [start, end] : ranges) {
        for (int code = start; code <= end; code++) {
            raw.emplace_back(code, loader.getGlyph(code));
        }
    }

    std::unique_ptr<char[]> pixels{new char[atlasSize.x * atlasSize.y]};
    auto dst = pixels.get();
    std::memset(pixels.get(), 0x00, atlasSize.x * atlasSize.y);

    Packer packer{raw.size(), atlasSize};
    for (auto& i : raw) {
        const auto code = std::get<0>(i);

        auto res = packer.add(std::get<1>(i).bitmapSize + padding * 2);
        if (!res) {
            EXCEPTION("Unable to fit glyph {} into the texture of size: {}", code, atlasSize);
        }

        const auto pos = *res + padding;
        const auto& data = std::get<1>(i);

        const auto src = data.bitmap.get();

        for (int row = 0; row < data.bitmapSize.y; row++) {
            for (int col = 0; col < data.bitmapSize.x; col++) {
                const auto x = pos.x + col;
                const auto y = pos.y + row;
                dst[y * atlasSize.x + x] = src[row * data.bitmapPitch + col];
            }
        }

        Glyph glyph{};
        glyph.advance = static_cast<float>(data.advance >> 6);
        glyph.uv = Vector2{
            static_cast<float>(pos.x + offset.x) / static_cast<float>(texture.getExtent().width),
            static_cast<float>(pos.y + offset.y) / static_cast<float>(texture.getExtent().height),
        };
        glyph.st = Vector2{
            static_cast<float>(data.bitmapSize.x) / static_cast<float>(texture.getExtent().width),
            static_cast<float>(data.bitmapSize.y) / static_cast<float>(texture.getExtent().height),
        };
        glyph.box = data.box;
        glyph.size = data.bitmapSize;
        glyph.ascend = data.ascend;

        glyphs.insert(std::make_pair(code, glyph));
    }

    vulkan.copyDataToImage(texture, 0, offset, 0, atlasSize, pixels.get());

    unknown = &glyphs.at(0);
}

FontFace::~FontFace() = default;

Vector2 FontFace::getBounds(const std::string_view& text, const float height) const {
    auto it = text.data();
    const auto end = it + text.size();

    Vector2 max{0.0f};
    Vector2 pos{0.0f};

    const auto scale = height / static_cast<float>(getSize());

    while (it < end) {
        const auto code = utf8::next(it, end);
        const auto& glyph = getGlyph(code);

        max.y = std::max(max.y, pos.y + height);
        pos += Vector2{glyph.advance * scale, 0.0f};
        max.x = std::max(max.x, pos.x);
    }

    return max;
}

Vector2 FontFace::getPenOffset(const std::string_view& text, float height, const TextAlign textAlign) const {
    const auto bounds = getBounds(text, height);

    switch (textAlign) {
    case TextAlign::Left: {
        return {0.0f, bounds.y / 2.0f};
    }
    case TextAlign::Center: {
        return {bounds.x / 2.0f, bounds.y / 2.0f};
    }
    case TextAlign::Right: {
        return {bounds.x, bounds.y / 2.0f};
    }
    case TextAlign::LeftTop: {
        return {0.0f, 0.0f};
    }
    case TextAlign::CenterTop: {
        return {bounds.x / 2.0f, 0.0f};
    }
    case TextAlign::RightTop: {
        return {bounds.x, 0.0f};
    }
    case TextAlign::LeftBottom: {
        return {0.0f, bounds.y};
    }
    case TextAlign::CenterBottom: {
        return {bounds.x / 2.0f, bounds.y};
    }
    case TextAlign::RightBottom: {
        return {bounds.x, bounds.y};
    }
    default: {
        return {0.0f, 0.0f};
    }
    }
}
