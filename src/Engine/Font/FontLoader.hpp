#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Path.hpp"
#include "../Utils/Span.hpp"

struct FT_FaceRec_;

namespace Engine {
class ENGINE_API FontLoader {
public:
    struct Glyph {
        std::unique_ptr<char[]> bitmap;
        Vector2i bitmapSize;
        Vector2 box;
        int advance;
        int bitmapPitch;
        float ascend;
    };

    explicit FontLoader(const Span<uint8_t>& data, int size);
    Glyph getGlyph(int code);

private:
    std::shared_ptr<FT_FaceRec_> face;
};
} // namespace Engine
