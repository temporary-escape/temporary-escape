#pragma once

#include "FontFace.hpp"

namespace Engine {
class ENGINE_API FontFamily {
public:
    static constexpr size_t total = 4;

    static_assert(FontFace::Type::Light + 1 == total);

    explicit FontFamily(VulkanRenderer& vulkan, const Path& dir, const std::string& name, float size);

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
