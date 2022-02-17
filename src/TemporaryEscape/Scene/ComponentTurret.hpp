#pragma once

#include "../Assets/AssetTurret.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentTurret : public Component {
public:
    struct Delta {
        Vector3 target;
        bool firing{false};

        MSGPACK_DEFINE_ARRAY(target, firing);
    };

    ComponentTurret() = default;
    explicit ComponentTurret(Object& object, AssetTurretPtr turret) : Component(object), turret(std::move(turret)) {
    }
    virtual ~ComponentTurret() = default;

    Delta getDelta() {
        Delta delta{};
        delta.target = target;
        delta.firing = firing;
        return delta;
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    const AssetTurretPtr& getTurret() const {
        return turret;
    }

    const Vector3& getTarget() const {
        return target;
    }

    void setTarget(const Vector3& value) {
        setDirty(true);
        target = value;
    }

    const Vector3& getRotation() const {
        return rotation;
    }

    const Vector3& getNozzlePos() const {
        return nozzlePos;
    }

    const Vector3& getNozzleDir() const {
        return nozzleDir;
    }

    bool shouldFire() const {
        if (!firing) {
            return false;
        }
        return counter <= 0.0f;
    }

    void resetFire() {
        counter = 0.1f;
    }

    void setFiring(bool value) {
        setDirty(true);
        firing = value;
    }

    bool getFiring() const {
        return firing;
    }

    void update(const float delta);

private:
    AssetTurretPtr turret;
    Vector3 target;
    bool firing{true};
    Vector3 rotation;
    Vector3 nozzleDir;
    Vector3 nozzlePos;
    float counter{0.0f};

public:
    MSGPACK_DEFINE_ARRAY(turret, target, firing);
};
} // namespace Engine
