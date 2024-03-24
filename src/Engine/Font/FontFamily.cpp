#include "FontFamily.hpp"
#include "TextShaper.hpp"

using namespace Engine;

Vector2 Engine::textAlignToVector(TextAlign textAlign) {
    switch (textAlign) {
    case TextAlign::Left: {
        return {0.0f, 0.5f};
    }
    case TextAlign::Center: {
        return {0.5f, 0.5f};
    }
    case TextAlign::Right: {
        return {1.0f, 0.5f};
    }
    case TextAlign::LeftTop: {
        return {0.0f, 0.0f};
    }
    case TextAlign::CenterTop: {
        return {0.5f, 0.0f};
    }
    case TextAlign::RightTop: {
        return {1.0f, 0.0f};
    }
    case TextAlign::LeftBottom: {
        return {0.0f, 1.0f};
    }
    case TextAlign::CenterBottom: {
        return {0.5f, 1.0f};
    }
    case TextAlign::RightBottom: {
        return {1.0f, 1.0f};
    }
    default: {
        return {0.0f, 0.0f};
    }
    }
}

FontFamily::FontFamily(VulkanRenderer& vulkan, const Sources& sources, const int size) :
    faces{
        FontFace{vulkan, sources.regular, size},
        FontFace{vulkan, sources.bold, size},
        FontFace{vulkan, sources.light, size},
    },
    size{size} {
}

Vector2 FontFamily::getBounds(const std::string_view& text, float height) const {
    TextShaper shaper{*this, height};
    shaper.write(text);
    return shaper.getBounds();
}

Vector2 FontFamily::getPenOffset(const std::string_view& text, float height, const TextAlign textAlign) const {
    return getBounds(text, height) * textAlignToVector(textAlign);
}
