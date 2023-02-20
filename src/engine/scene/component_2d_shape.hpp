#pragma once

#include "../assets/image.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API Component2DShape : public Component {
public:
    enum class Type {
        None,
        Circle,
    };

    Component2DShape() = default;
    explicit Component2DShape(const Type type, const Color4& color) : type{type}, color{color} {
    }
    virtual ~Component2DShape() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(Component2DShape);

    [[nodiscard]] Type getType() const {
        return type;
    }

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

private:
    Type type;
    Color4 color;
};
} // namespace Engine
