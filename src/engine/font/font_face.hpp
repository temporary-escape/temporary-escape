#pragma once

#include "../utils/path.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include <unordered_map>

namespace Engine {
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
