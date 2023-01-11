#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentTurret : public Component {
public:
    ComponentTurret() = default;
    virtual ~ComponentTurret() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentTurret);

    [[nodiscard]] const Vector3& getTarget() const {
        return target;
    }

    void setTarget(const Vector3& value) {
        setDirty(true);
        target = value;
    }

    [[nodiscard]] const Vector3& getRotation() const {
        return rotation;
    }

    [[nodiscard]] const Vector3& getNozzlePos() const {
        return nozzlePos;
    }

    [[nodiscard]] const Vector3& getNozzleDir() const {
        return nozzleDir;
    }

    [[nodiscard]] bool shouldFire() const {
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

    [[nodiscard]] bool getFiring() const {
        return firing;
    }

    void update(float delta);

private:
    Vector3 target;
    bool firing{true};
    Vector3 rotation;
    Vector3 nozzleDir;
    Vector3 nozzlePos;
    float counter{0.0f};
};
} // namespace Engine
