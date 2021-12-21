#pragma once

#include "../Assets/AssetModel.hpp"
#include "Component.hpp"

namespace Scissio {
class SCISSIO_API ComponentModel : public Component {
public:
    static constexpr ComponentType Type = 2;

    ComponentModel();
    explicit ComponentModel(Object& object, AssetModelPtr model);
    virtual ~ComponentModel() = default;

    const AssetModelPtr& getAssetModel() const {
        return model;
    }

private:
    AssetModelPtr model{nullptr};

public:
    MSGPACK_DEFINE_ARRAY(model);
};
} // namespace Scissio
