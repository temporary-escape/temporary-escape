#pragma once

#include "FontFace.hpp"

namespace Engine {
class ENGINE_API FontFamily {
public:
    struct Sources {
        Span<uint8_t> regular;
        Span<uint8_t> bold;
        Span<uint8_t> light;
        Span<uint8_t> thin;
    };

    static constexpr size_t total = 4;

    static_assert(FontFace::Type::Light + 1 == total);

    explicit FontFamily(VulkanRenderer& vulkan, const Sources& sources, int size);

    FontFace& get(FontFace::Type type) {
        return faces.at(type);
    }

    const FontFace& get(const FontFace::Type type) const {
        return faces.at(type);
    }

private:
    std::array<FontFace, total> faces;
};
} // namespace Engine
