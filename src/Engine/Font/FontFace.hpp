#pragma once

#include "../Utils/Path.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include <unordered_map>

namespace Engine {
enum class TextAlign {
    Left = 0,
    Center,
    Right,
    LeftTop,
    CenterTop,
    RightTop,
    LeftBottom,
    CenterBottom,
    RightBottom,
};

class ENGINE_API FontFace {
public:
    static inline const Vector2i atlasSize{2048, 1024};

    struct Glyph {
        Vector2 uv;
        Vector2 st;
        Vector2 size;
        Vector2 box;
        float advance;
        float ascend;
    };

    explicit FontFace(VulkanRenderer& vulkan, const Span<uint8_t>& source, VulkanTexture& texture,
                      const Vector2i& offset, int size);
    ~FontFace();

    const Glyph& getGlyph(const uint32_t code) const {
        const auto it = glyphs.find(code);
        if (it == glyphs.end()) {
            return *unknown;
        }
        return it->second;
    }

    int getSize() const {
        return size;
    }

    Vector2 getBounds(const std::string_view& text, float height) const;
    Vector2 getPenOffset(const std::string_view& text, float height, const TextAlign textAlign) const;

private:
    static inline const Vector2i padding{2, 2};
    const int size;
    std::unordered_map<uint32_t, Glyph> glyphs;
    const Glyph* unknown;
};
} // namespace Engine
