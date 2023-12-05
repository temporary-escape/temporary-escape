#pragma once

#include "../Component.hpp"

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

    MSGPACK_DEFINE_ARRAY(label);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    std::string label;
};
} // namespace Engine
