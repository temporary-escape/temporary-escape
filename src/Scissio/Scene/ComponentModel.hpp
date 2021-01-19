#pragma once

#include "../Assets/Model.hpp"
#include "../Library.hpp"
#include "ComponentSystem.hpp"

namespace Scissio {
class Renderer;

class SCISSIO_API ComponentModel : public Component {
public:
    static constexpr ComponentType Type = 1;

    ComponentModel() = default;
    explicit ComponentModel(Object& object, ModelPtr model) : Component(Type, object), model(std::move(model)) {
    }
    virtual ~ComponentModel() = default;

    const ModelPtr& getModel() const {
        return model;
    }

private:
    ModelPtr model{nullptr};
};
} // namespace Scissio
