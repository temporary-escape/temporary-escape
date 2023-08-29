#include "controller_grid.hpp"

using namespace Engine;

ControllerGrid::ControllerGrid(entt::registry& reg, VoxelShapeCache& voxelShapeCache) :
    reg{reg}, voxelShapeCache{voxelShapeCache} {
}

ControllerGrid::~ControllerGrid() = default;

void ControllerGrid::update(const float delta) {
    (void)delta;
}

void ControllerGrid::recalculate(VulkanRenderer& vulkan) {
    for (auto&& [_, grid] : reg.view<ComponentGrid>().each()) {
        if (grid.isDirty()) {
            grid.setDirty(false);
            grid.recalculate(vulkan, voxelShapeCache);
        }
    }
}
