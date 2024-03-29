#pragma once

#include "../Controller.hpp"
#include "../DynamicsWorld.hpp"

namespace Engine {
class ENGINE_API ControllerBullets : public Controller {
public:
    explicit ControllerBullets(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld);
    ~ControllerBullets() override;
    NON_COPYABLE(ControllerBullets);
    NON_MOVEABLE(ControllerBullets);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanDoubleBuffer& getVbo() const {
        return vbo;
    }
    size_t getCount() const {
        return bullets.size();
    }

    ComponentTurret::BulletInstance& allocateBullet();

private:
    Scene& scene;
    entt::registry& reg;
    DynamicsWorld& dynamicsWorld;

    std::vector<ComponentTurret::BulletInstance> bullets;
    VulkanDoubleBuffer vbo;
};
} // namespace Engine
