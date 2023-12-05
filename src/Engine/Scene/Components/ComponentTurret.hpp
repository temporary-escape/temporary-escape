#pragma once

#include "../../Assets/Turret.hpp"
#include "ComponentModelSkinned.hpp"
#include "ComponentTransform.hpp"

namespace Engine {
class ENGINE_API ComponentTurret : public Component {
public:
    struct ENGINE_API BulletInstance {
        Vector3 origin;
        Vector3 direction;
        float lifetime{0.0f};
        float size;
        float speed;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(BulletInstance, origin)},
                {2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(BulletInstance, direction)},
                {3, VK_FORMAT_R32_SFLOAT, offsetof(BulletInstance, lifetime)},
                {4, VK_FORMAT_R32_SFLOAT, offsetof(BulletInstance, size)},
                {5, VK_FORMAT_R32_SFLOAT, offsetof(BulletInstance, speed)},
            };
        };
    };

    ComponentTurret() = default;
    explicit ComponentTurret(entt::registry& reg, entt::entity handle);
    explicit ComponentTurret(entt::registry& reg, entt::entity handle, TurretPtr turret);
    virtual ~ComponentTurret() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentTurret);

    void update(float delta, const ComponentTransform& transform, ComponentModelSkinned& model);

    void setTurret(TurretPtr value);
    const TurretPtr& getTurret() const {
        return turret;
    }

    const Vector3& getTargetPos() const {
        return targetPos;
    }

    const Vector3& getTargetDirection() const {
        return targetDirGlobal;
    }

    void setTarget(const ComponentTransform* value);
    const ComponentTransform* getTarget() const {
        return target;
    }

    void clearTarget();

    bool isActive() const {
        return active;
    }

    int getCounter() const {
        return counter;
    }

    bool shouldShoot() const;
    void resetShoot();

    MSGPACK_DEFINE_ARRAY(turret, targetPos, active, shootReady);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    TurretPtr turret;
    Vector3 targetPos;
    Vector3 targetDirGlobal{0.0f, 0.0f, -1.0f};
    bool active{false};
    int counter{0};
    bool shootReady{false};
    const ComponentTransform* target{nullptr};
};
} // namespace Engine
