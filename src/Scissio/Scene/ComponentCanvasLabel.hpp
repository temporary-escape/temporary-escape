#pragma once

#include "../Assets/AssetFontFace.hpp"
#include "../Library.hpp"
#include "Component.hpp"

namespace Scissio {
class SCISSIO_API ComponentCanvasLabel : public Component {
public:
    ComponentCanvasLabel() = default;
    explicit ComponentCanvasLabel(Object& object, AssetFontFacePtr fontFace, std::string text, const Color4& color,
                                  const float size)
        : Component(object), fontFace(std::move(fontFace)), text(std::move(text)), color(color), size(size) {
    }
    virtual ~ComponentCanvasLabel() = default;

    const AssetFontFacePtr& getFontFace() const {
        return fontFace;
    }

    void setFontFace(AssetFontFacePtr value) {
        fontFace = std::move(value);
    }

    const std::string& getText() const {
        return text;
    }

    void setText(std::string value) {
        text = std::move(value);
    }

    const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        color = value;
    }

    bool getVisible() const {
        return visible;
    }

    void setVisible(const bool value) {
        visible = value;
    }

    float getSize() const {
        return size;
    }

    void setSize(const float value) {
        size = value;
    }

    bool getCentered() const {
        return centered;
    }

    void setCentered(const bool value) {
        centered = value;
    }

    const Vector2& getOffset() const {
        return offset;
    }
    void setOffset(const Vector2& offset) {
        ComponentCanvasLabel::offset = offset;
    }

private:
    AssetFontFacePtr fontFace;
    std::string text;
    Color4 color;
    float size{18.0f};
    bool visible{true};
    bool centered{false};
    Vector2 offset{0.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Scissio
