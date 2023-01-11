#pragma once

#include "../library.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentText : public Component {
public:
    ComponentText() = default;
    explicit ComponentText(std::string text, const Color4& color, const float size) :
        text{std::move(text)}, color{color}, size{size} {
    }
    virtual ~ComponentText() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentText);

    [[nodiscard]] const std::string& getText() const {
        return text;
    }

    void setText(std::string value) {
        text = std::move(value);
    }

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        color = value;
    }

    [[nodiscard]] bool getVisible() const {
        return visible;
    }

    void setVisible(const bool value) {
        visible = value;
    }

    [[nodiscard]] float getSize() const {
        return size;
    }

    void setSize(const float value) {
        size = value;
    }

    [[nodiscard]] bool getCentered() const {
        return centered;
    }

    void setCentered(const bool value) {
        centered = value;
    }

    [[nodiscard]] const Vector2& getOffset() const {
        return offset;
    }

    void setOffset(const Vector2& value) {
        offset = value;
    }

private:
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
