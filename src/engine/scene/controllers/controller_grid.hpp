#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerGrid : public Controller {
public:
    explicit ControllerGrid(entt::registry& reg, VoxelShapeCache& voxelShapeCache);
    ~ControllerGrid() override;
    NON_COPYABLE(ControllerGrid);
    NON_MOVEABLE(ControllerGrid);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
    VoxelShapeCache& voxelShapeCache;
};
} // namespace Engine
