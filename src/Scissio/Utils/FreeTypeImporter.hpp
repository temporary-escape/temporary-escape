#pragma once

#include "../Math/Vector.hpp"
#include "Path.hpp"

namespace Scissio {
struct FreeTypeGlyph {
    std::unique_ptr<char[]> pixels;
    Vector2i size;
    Vector2i advance;
};

class FreeTypeImporter {
public:
    explicit FreeTypeImporter(const Path& path);
    ~FreeTypeImporter();

    void setSize(int pixels);
    FreeTypeGlyph getGlyph(int charcode);

private:
    struct Face;
    std::unique_ptr<Face> face;
};
} // namespace Scissio
