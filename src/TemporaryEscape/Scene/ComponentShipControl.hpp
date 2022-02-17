#pragma once

#include "ComponentGrid.hpp"

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API ComponentShipControl : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentShipControl() {
    }
    explicit ComponentShipControl(Object& object) : Component(object) {
    }
    virtual ~ComponentShipControl() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void init();
    void update(float delta);

private:
    void initParticles(const std::shared_ptr<Entity>& entity, const ComponentGrid& grid);
    [[nodiscard]] std::shared_ptr<Entity> getEntity() const;

public:
    bool enabled{false};

    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
