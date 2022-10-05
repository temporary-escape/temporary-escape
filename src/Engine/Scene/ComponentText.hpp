#pragma once

#include "../Library.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentText : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentText() = default;
    explicit ComponentText(Object& object, std::string text, const Color4& color, const float size) :
        Component(object), text{std::move(text)}, color{color}, size{size} {
    }
    virtual ~ComponentText() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    /*const AssetFontFacePtr& getFontFace() const {
        return fontFace;
    }

    void setFontFace(AssetFontFacePtr value) {
        fontFace = std::move(value);
    }*/

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
        ComponentText::offset = offset;
    }

private:
    // AssetFontFacePtr fontFace;
    std::string text;
    Color4 color;
    float size{18.0f};
    bool visible{true};
    bool centered{false};
    Vector2 offset{0.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
