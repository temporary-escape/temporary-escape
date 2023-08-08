#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerPointCloud : public Controller {
public:
    explicit ControllerPointCloud(entt::registry& reg);
    ~ControllerPointCloud() override;
    NON_COPYABLE(ControllerPointCloud);
    NON_MOVEABLE(ControllerPointCloud);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
};
} // namespace Engine
