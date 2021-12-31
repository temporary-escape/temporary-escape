#pragma once

#include "../Assets/AssetModel.hpp"
#include "Component.hpp"

namespace Scissio {
class SCISSIO_API ComponentModel : public Component {
public:
    ComponentModel() = default;
    explicit ComponentModel(Object& object, AssetModelPtr model) : Component(object), model(std::move(model)) {
    }

    virtual ~ComponentModel() = default;

    const AssetModelPtr& getModel() const {
        return model;
    }

private:
    AssetModelPtr model{nullptr};

public:
    MSGPACK_DEFINE_ARRAY(model);
};
} // namespace Scissio
