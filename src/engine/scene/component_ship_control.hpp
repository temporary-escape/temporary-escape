#pragma once

#include "component.hpp"

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
    void setMovement(bool left, bool right, bool up, bool down) {
        moving = left || right || up || down;
        rotate[0] = left;
        rotate[1] = right;
        rotate[2] = up;
        rotate[3] = down;
    }

    std::tuple<float, float, float> getAngles() const;

private:
    // void initParticles(const std::shared_ptr<Entity>& entity, const ComponentGrid& grid);
    [[nodiscard]] std::shared_ptr<Entity> getEntity() const;

    bool moving{true};
    bool rotate[4] = {false};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
