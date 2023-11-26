#include "FontLoader.hpp"
#include "../Utils/Exceptions.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static FT_Library initFreeType() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        EXCEPTION("FT_Init_FreeType failed");
    }

    return ft;
}

static FT_Library getFreeType() {
    static std::shared_ptr<FT_LibraryRec_> ft =
        std::shared_ptr<FT_LibraryRec_>(initFreeType(), [](FT_Library l) { FT_Done_FreeType(l); });

    return ft.get();
}

FontLoader::FontLoader(const Path& path, const float size) {
    auto ft = getFreeType();

    FT_Face f;
    const auto pathStr = path.string();
    if (FT_New_Face(ft, pathStr.c_str(), 0, &f)) {
        EXCEPTION("Failed to open font: '{}'", path);
    }

    face = std::shared_ptr<FT_FaceRec_>(f, [](FT_Face f) { FT_Done_Face(f); });

    FT_Set_Pixel_Sizes(f, 0, static_cast<FT_UInt>(size));
}

FontLoader::Glyph FontLoader::getGlyph(int code) {
    Glyph glyph{};

    // Load character glyph
    if (FT_Load_Char(face.get(), code, FT_LOAD_RENDER)) {
        EXCEPTION("Failed to load glyph: {}", code);
    }

    // glyph.bearing.x = face->glyph->bitmap_left;
    // glyph.bearing.x = face->glyph->bitmap_top;
    glyph.box.x = static_cast<float>(face->glyph->metrics.width) / static_cast<float>(1 << 6);
    glyph.box.y = static_cast<float>(face->glyph->metrics.height) / static_cast<float>(1 << 6);
    glyph.bitmapSize.x = face->glyph->bitmap.width;
    glyph.bitmapSize.y = face->glyph->bitmap.rows;
    glyph.bitmapPitch = face->glyph->bitmap.pitch;
    glyph.advance = face->glyph->advance.x;
    glyph.ascend = face->glyph->bitmap.rows - face->glyph->bitmap_top;

    glyph.bitmap = std::make_unique<char[]>(glyph.bitmapSize.x * glyph.bitmapSize.y);
    std::memcpy(glyph.bitmap.get(), face->glyph->bitmap.buffer, glyph.bitmapSize.x * glyph.bitmapSize.y);

    return glyph;
}
