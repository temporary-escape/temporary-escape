#pragma once

#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentDirectionalLight : public Component {
public:
    ComponentDirectionalLight() = default;
    explicit ComponentDirectionalLight(EntityId entity, const Color4& color);
    COMPONENT_DEFAULTS(ComponentDirectionalLight);

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        color = value;
    }

private:
    Color4 color{1.0f};
};
} // namespace Engine
