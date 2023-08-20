#pragma once

#include "../../assets/primitive.hpp"
#include "../component.hpp"
#include "../grid.hpp"
#include "component_debug.hpp"

namespace Engine {
class ENGINE_API ComponentGrid : public Component, public Grid {
public:
    ComponentGrid() = default;
    explicit ComponentGrid(entt::registry& reg, entt::entity handle);
    virtual ~ComponentGrid();
    COMPONENT_DEFAULTS(ComponentGrid);

    void clear();
    void recalculate(VulkanRenderer& vulkan, const VoxelShapeCache& voxelShapeCache);
    void update();

    [[nodiscard]] const std::list<Primitive>& getPrimitives() const {
        return primitives;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Grid));

private:
    void debugIterate(Grid::Iterator iterator);

    ComponentDebug* debug{nullptr};
    VulkanRenderer* vulkanRenderer{nullptr};
    std::list<Primitive> primitives;
};
} // namespace Engine
