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
    struct Glyph {
        Vector2 uv;
        Vector2 st;
        Vector2 size;
        Vector2 box;
        float advance;
        float ascend;
    };

    enum Type {
        Regular = 0,
        Bold,
        Thin,
        Light,
    };

    explicit FontFace(VulkanRenderer& vulkan, const Path& path, float size);
    ~FontFace();

    const Glyph& getGlyph(const uint32_t code) const {
        const auto it = glyphs.find(code);
        if (it == glyphs.end()) {
            return *unknown;
        }
        return it->second;
    }

    float getSize() const {
        return size;
    }

    Vector2 getBounds(const std::string_view& text, float height) const;
    Vector2 getPenOffset(const std::string_view& text, float height, const TextAlign textAlign) const;

    [[nodiscard]] VulkanTexture& getTexture() {
        return texture;
    }

    [[nodiscard]] const VulkanTexture& getTexture() const {
        return texture;
    }

private:
    static inline const Vector2i padding{2, 2};
    const float size;
    std::unordered_map<uint32_t, Glyph> glyphs;
    const Glyph* unknown;
    VulkanTexture texture;
};
} // namespace Engine