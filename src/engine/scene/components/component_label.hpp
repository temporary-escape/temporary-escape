#pragma once

#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentLabel : public Component {
public:
    ComponentLabel() = default;
    explicit ComponentLabel(entt::registry& reg, entt::entity handle, std::string label);

    virtual ~ComponentLabel() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentLabel);

    [[nodiscard]] const std::string& getLabel() const {
        return label;
    }

    void setLabel(std::string value) {
        label = std::move(value);
        setDirty(true);
    }

private:
    std::string label;
    Color4 color;
};
} // namespace Engine
