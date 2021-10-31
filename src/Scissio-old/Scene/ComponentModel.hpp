#pragma once

#include "../Assets/Model.hpp"
#include "Component.hpp"

namespace Scissio {
class ShaderModel;

class SCISSIO_API ComponentModel : public Component {
public:
    static constexpr ComponentType Type = 2;

    ComponentModel();
    explicit ComponentModel(Object& object, ModelPtr model);
    virtual ~ComponentModel() = default;

    const ModelPtr& getModel() const {
        return model;
    }

    void setModel(ModelPtr model) {
        this->model = std::move(model);
    }

    void render(ShaderModel& shader);

private:
    ModelPtr model{nullptr};

public:
    MSGPACK_DEFINE_ARRAY(model);
};
} // namespace Scissio
