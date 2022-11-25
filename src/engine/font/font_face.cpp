#include "font_face.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/packer.hpp"
#include "font_loader.hpp"
#include <utf8.h>

#define CMP "FontFace"

using namespace Engine;

FontFace::FontFace(VulkanDevice& vulkan, const Path& path, const float size) : size{size} {
    auto loader = FontLoader(path, size);

    std::vector<std::tuple<int, FontLoader::Glyph>> raw;

    static std::vector<std::tuple<int, int>> ranges = {
        {0x0000, 0x0000}, // Unknown,
        {0x0020, 0x007F}, // Basic Latin
        {0x00A0, 0x00FF}, // Latin-1 Supplement
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

    std::vector<Vector2i> positions;
    positions.resize(raw.size());

    int finalWidth = 0;

    for (int width = 512; width <= 8192; width = width << 1) {
        Packer packer{raw.size(), {width, width}};
        bool failed = false;

        for (size_t i = 0; i < raw.size(); i++) {
            auto res = packer.add(std::get<1>(raw[i]).bitmapSize + padding * 2);
            if (!res) {
                failed = true;
                break;
            }

            positions[i] = *res + padding;
        }

        if (!failed) {
            finalWidth = width;
            break;
        }
    }

    if (finalWidth == 0) {
        EXCEPTION("Unable to pack all glyphs for font: '{}' size: {}", path, size);
    }

    std::unique_ptr<char[]> pixels(new char[finalWidth * finalWidth]);
    auto dst = pixels.get();
    std::memset(pixels.get(), 0x00, finalWidth * finalWidth);

    for (size_t i = 0; i < raw.size(); i++) {
        const auto& data = std::get<1>(raw[i]);
        const auto code = std::get<0>(raw[i]);
        const auto pos = positions[i];

        const auto src = data.bitmap.get();

        for (int row = 0; row < data.bitmapSize.y; row++) {
            for (int col = 0; col < data.bitmapSize.x; col++) {
                const auto x = pos.x + col;
                const auto y = pos.y + row;
                dst[y * finalWidth + x] = src[row * data.bitmapPitch + col];
            }
        }

        Glyph glyph{};
        glyph.advance = static_cast<float>(data.advance >> 6);
        glyph.uv = Vector2{
            static_cast<float>(pos.x) / static_cast<float>(finalWidth),
            static_cast<float>(pos.y) / static_cast<float>(finalWidth),
        };
        glyph.st = Vector2{
            static_cast<float>(data.bitmapSize.x) / static_cast<float>(finalWidth),
            static_cast<float>(data.bitmapSize.y) / static_cast<float>(finalWidth),
        };
        glyph.box = data.box;
        glyph.size = data.bitmapSize;
        glyph.ascend = data.ascend;

        glyphs.insert(std::make_pair(code, glyph));
    }

    unknown = &glyphs.at(0);

    VulkanTexture::Descriptor desc{};
    desc.format = VulkanTexture::Format::VK_FORMAT_R8_UNORM;
    desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    desc.size = {finalWidth, finalWidth};

    texture = vulkan.createTexture(desc);
    texture.subData(0, {0, 0}, desc.size, pixels.get());

    Log::i(CMP, "Loaded font: '{}' of size: {} with atlas size: {}", path, size, Vector2i{finalWidth});
}

FontFace::~FontFace() {
}

Vector2 FontFace::getBounds(const std::string_view& text, const float height) const {
    auto it = text.data();
    const auto end = it + text.size();

    size_t total = 0;
    Vector2 max{0.0f};
    Vector2 pos{0.0f};

    const auto scale = height / getSize();

    while (it < end) {
        const auto code = utf8::next(it, end);
        const auto& glyph = getGlyph(code);

        max.y = std::max(max.y, pos.y + height);
        max.x = std::max(max.x, pos.x + glyph.box.x * scale);

        pos += Vector2{glyph.advance * scale, 0.0f};
    }

    return max;
}
