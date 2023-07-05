#pragma once

#include "../../assets/image.hpp"
#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentIcon : public Component {
public:
    ComponentIcon() = default;
    explicit ComponentIcon(entt::registry& reg, entt::entity handle, ImagePtr image, const Vector2& size,
                           const Color4& color);
    virtual ~ComponentIcon() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentIcon);

    const ImagePtr& getImage() const {
        return image;
    }

    void setImage(const ImagePtr& value) {
        image = value;
    }

    const Vector2& getSize() const {
        return size;
    }

    void setSize(const Vector2& value) {
        size = value;
    }

    const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        color = value;
    }

    const Vector2& getOffset() const {
        return offset;
    }

    void setOffset(const Vector2& value) {
        offset = value;
    }

private:
    ImagePtr image;
    Vector2 size;
    Color4 color;
    Vector2 offset{0.0f};
};
} // namespace Engine
