#pragma once

#include "../../assets/turret.hpp"
#include "component_model_skinned.hpp"
#include "component_transform.hpp"

namespace Engine {
class ENGINE_API ComponentTurret : public Component {
public:
    ComponentTurret() = default;
    explicit ComponentTurret(entt::registry& reg, entt::entity handle);
    explicit ComponentTurret(entt::registry& reg, entt::entity handle, TurretPtr turret);
    virtual ~ComponentTurret() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentTurret);

    /*[[nodiscard]] const Vector3& getTarget() const {
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
    }*/

    void update(float delta, const ComponentTransform& transform, ComponentModelSkinned& model);

    void setTurret(TurretPtr value);
    const TurretPtr& getTurret() const {
        return turret;
    }

    const Vector3& getTargetPos() const {
        return targetPos;
    }

    void setTarget(const ComponentTransform* value);
    const ComponentTransform* getTarget() const {
        return target;
    }

    void clearTarget();

    bool isActive() const {
        return active;
    }

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(turret, targetPos, active);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    TurretPtr turret;
    Vector3 targetPos;
    bool active{false};
    const ComponentTransform* target{nullptr};

    /*Vector3 target;
    bool firing{true};
    Vector3 rotation;
    Vector3 nozzleDir;
    Vector3 nozzlePos;
    float counter{0.0f};*/
};
} // namespace Engine
