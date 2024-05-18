#pragma once

#include "FontFace.hpp"

namespace Engine {
ENGINE_API Vector2 textAlignToVector(TextAlign textAlign);

enum class FontType {
    Regular = 0,
    RegularItalic,
    Bold,
    BoldItalic,
    Light,
    LightItalic,
};

class ENGINE_API FontFamily {
public:
    static constexpr size_t total = 6;
    static_assert(static_cast<int>(FontType::LightItalic) + 1 == total);

    using Sources = std::array<Path, total>;

    explicit FontFamily(VulkanRenderer& vulkan, const Sources& sources, int size);
    virtual ~FontFamily() = default;

    FontFace& get(const FontType type) {
        return *faces.at(static_cast<size_t>(type));
    }

    [[nodiscard]] const FontFace& get(const FontType type) const {
        return *faces.at(static_cast<size_t>(type));
    }

    int getSize() const {
        return size;
    }

    [[nodiscard]] float getGlyphAdvance(uint32_t code, float height) const;
    [[nodiscard]] Vector2 getBounds(const std::string_view& text, float height) const;
    [[nodiscard]] Vector2 getPenOffset(const std::string_view& text, float height, TextAlign textAlign) const;

    [[nodiscard]] VulkanTexture& getTexture() {
        return texture;
    }

    [[nodiscard]] const VulkanTexture& getTexture() const {
        return texture;
    }

private:
    static inline const Vector2i atlasSize{4096, 4096};
    std::array<std::unique_ptr<FontFace>, total> faces;
    int size;
    VulkanTexture texture;
};
} // namespace Engine
