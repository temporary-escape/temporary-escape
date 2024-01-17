#pragma once

#include "../../Font/FontFace.hpp"
#include "../../Library.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentText : public Component {
public:
    ComponentText() = default;
    explicit ComponentText(EntityId entity, std::string text, const Color4& color, const float size);
    virtual ~ComponentText() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentText);

    void recalculate(FontFace& font);

    [[nodiscard]] const std::string& getText() const {
        return text;
    }

    void setText(std::string value) {
        dirty = true;
        text = std::move(value);
    }

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        dirty = true;
        color = value;
    }

    [[nodiscard]] float getSize() const {
        return size;
    }

    void setSize(const float value) {
        dirty = true;
        size = value;
    }

    [[nodiscard]] bool getCentered() const {
        return centered;
    }

    void setCentered(const bool value) {
        dirty = true;
        centered = value;
    }

    [[nodiscard]] const Vector2& getOffset() const {
        return offset;
    }

    void setOffset(const Vector2& value) {
        dirty = true;
        offset = value;
    }

    const Vector2& getBounds() const {
        return bounds;
    }

private:
    bool dirty{false};
    std::string text;
    Color4 color;
    float size{18.0f};
    bool centered{false};
    Vector2 offset{0.0f};
    Vector2 bounds{};
};
} // namespace Engine
