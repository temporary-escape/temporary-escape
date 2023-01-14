#pragma once

#include "../assets/image.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentIcon : public Component {
public:
    ComponentIcon() = default;
    explicit ComponentIcon(const ImagePtr& image, const Vector2& size, const Color4& color) :
        image{image}, size{size}, color{color} {
    }
    virtual ~ComponentIcon() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentIcon);

    const ImagePtr& getImage() const {
        return image;
    }
    const Vector2& getSize() const {
        return size;
    }
    const Color4& getColor() const {
        return color;
    }

private:
    ImagePtr image;
    Vector2 size;
    Color4 color;
};
} // namespace Engine
