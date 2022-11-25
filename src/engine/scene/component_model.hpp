#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentModel : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentModel() = default;
    explicit ComponentModel(Object& object) : Component(object) {
    }

    virtual ~ComponentModel() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    /*const AssetModelPtr& getModel() const {
        return model;
    }*/

private:
    // AssetModelPtr model{nullptr};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
