#include <ft2build.h>
#include FT_FREETYPE_H
#include "Exceptions.hpp"
#include "FreeTypeImporter.hpp"

using namespace Scissio;

class FreeTypeLibrary {
public:
    FreeTypeLibrary() {
        const auto error = FT_Init_FreeType(&library);
        if (error) {
            EXCEPTION("Failed to initialize FreeType library");
        }
    }

    ~FreeTypeLibrary() {
        if (library) {
            FT_Done_FreeType(library);
        }
    }

    FT_Library library;
};

static std::unique_ptr<FreeTypeLibrary> ft = std::make_unique<FreeTypeLibrary>();

struct FreeTypeImporter::Face {
    FT_Face face;
};

FreeTypeImporter::FreeTypeImporter(const Path& path) : face(std::make_unique<Face>()) {
    const auto pathStr = path.string();
    const auto error = FT_New_Face(ft->library, pathStr.c_str(), 0, &face->face);

    if (error == FT_Err_Unknown_File_Format) {
        EXCEPTION("Failed to open font, unknown file format");
    } else if (error) {
        EXCEPTION("Failed to open font, unknwon error");
    }
}

FreeTypeImporter::~FreeTypeImporter() {
    if (face && face->face) {
        FT_Done_Face(face->face);
    }
}

void FreeTypeImporter::setSize(int pixels) {
    const auto error = FT_Set_Pixel_Sizes(face->face, /* handle to face object */
                                          0,          /* pixel_width           */
                                          16);        /* pixel_height          */
    if (error) {
        EXCEPTION("Failed to set pixel size for font face");
    }
}

FreeTypeGlyph FreeTypeImporter::getGlyph(int charcode) {
    const auto glyphIndex = FT_Get_Char_Index(face->face, charcode);

    auto error = FT_Load_Glyph(face->face,       /* handle to face object */
                               glyphIndex,       /* glyph index           */
                               FT_LOAD_DEFAULT); /* load flags, see below */

    if (error) {
        EXCEPTION("Failed to load glyph for code {}", charcode);
    }

    error = FT_Render_Glyph(face->face->glyph,      /* glyph slot  */
                            FT_RENDER_MODE_NORMAL); /* render mode */

    if (error) {
        EXCEPTION("Failed to render glyph for code {}", charcode);
    }

    auto bitmap = face->face->glyph->bitmap;
    FreeTypeGlyph glyph;
    // glyph.pixels = bitmap.buffer;
    glyph.pixels.reset(new char[bitmap.width * bitmap.rows]);
    std::memcpy(glyph.pixels.get(), bitmap.buffer, bitmap.width * bitmap.rows);
    glyph.size = {bitmap.width, bitmap.rows};
    glyph.advance = {face->face->glyph->advance.x, face->face->glyph->advance.y};
    return glyph;
}
