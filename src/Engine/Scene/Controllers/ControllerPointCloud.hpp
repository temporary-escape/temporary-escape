#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerPointCloud : public Controller {
public:
    explicit ControllerPointCloud(Scene& scene, entt::registry& reg);
    ~ControllerPointCloud() override;
    NON_COPYABLE(ControllerPointCloud);
    NON_MOVEABLE(ControllerPointCloud);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    Scene& scene;
    entt::registry& reg;
};
} // namespace Engine
