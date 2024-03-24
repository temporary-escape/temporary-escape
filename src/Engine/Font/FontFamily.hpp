#pragma once

#include "FontFace.hpp"

namespace Engine {
ENGINE_API Vector2 textAlignToVector(TextAlign textAlign);

class ENGINE_API FontFamily {
public:
    struct Sources {
        Span<uint8_t> regular;
        Span<uint8_t> bold;
        Span<uint8_t> light;
    };

    static constexpr size_t total = 3;

    static_assert(FontFace::Type::Light + 1 == total);

    explicit FontFamily(VulkanRenderer& vulkan, const Sources& sources, int size);

    FontFace& get(FontFace::Type type) {
        return faces.at(type);
    }

    const FontFace& get(const FontFace::Type type) const {
        return faces.at(type);
    }

    int getSize() const {
        return size;
    }

    Vector2 getBounds(const std::string_view& text, float height) const;
    Vector2 getPenOffset(const std::string_view& text, float height, TextAlign textAlign) const;

private:
    std::array<FontFace, total> faces;
    int size;
};
} // namespace Engine
