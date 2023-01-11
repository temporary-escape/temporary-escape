#pragma once

#include "../assets/model.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentModel : public Component {
public:
    ComponentModel() = default;
    explicit ComponentModel(ModelPtr model) : model{std::move(model)} {
    }

    virtual ~ComponentModel() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentModel);

    void setModel(ModelPtr value) {
        model = std::move(value);
    }

    [[nodiscard]] const ModelPtr& getModel() const {
        return model;
    }

private:
    ModelPtr model{nullptr};
};
} // namespace Engine
