#pragma once

#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentLabel : public Component {
public:
    ComponentLabel() = default;
    explicit ComponentLabel(EntityId entity, std::string label);

    virtual ~ComponentLabel() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentLabel);

    [[nodiscard]] const std::string& getLabel() const {
        return label;
    }

    void setLabel(std::string value) {
        label = std::move(value);
    }

    MSGPACK_DEFINE_ARRAY(label);

private:
    std::string label;
};
} // namespace Engine
