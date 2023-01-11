#pragma once

#include "../assets/primitive.hpp"
#include "component.hpp"
#include "component_debug.hpp"
#include "grid.hpp"

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API ComponentGrid : public Component, public Grid {
public:
    ComponentGrid() = default;
    explicit ComponentGrid(ComponentDebug& debug) : debug{&debug} {
    }
    virtual ~ComponentGrid() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentGrid);

    void recalculate(VulkanRenderer& vulkan, const VoxelShapeCache& voxelShapeCache);
    void update();

    [[nodiscard]] const std::vector<Primitive>& getPrimitives() const {
        return primitives;
    }

private:
    void debugIterate(Grid::Iterator iterator);

    ComponentDebug* debug{nullptr};
    std::vector<Primitive> primitives;
};
} // namespace Engine
