#pragma once

#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentPointLight : public Component {
public:
    ComponentPointLight() = default;
    explicit ComponentPointLight(entt::registry& reg, entt::entity handle, const Color4& color);
    virtual ~ComponentPointLight() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPointLight);

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        color = value;
    }

    [[nodiscard]] bool isInfinite() const {
        return infinite;
    }

    void setInfinite(const bool value) {
        infinite = value;
    }

private:
    Color4 color{1.0f};
    bool infinite{false};
};
} // namespace Engine
