#pragma once

#include "../Assets/Primitive.hpp"
#include "Component.hpp"
#include "ComponentDebug.hpp"
#include "Grid.hpp"

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API ComponentGrid : public Component, public Grid {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentGrid() {
    }
    explicit ComponentGrid(Object& object, ComponentDebug* debug = nullptr) : Component{object}, debug{debug} {
    }
    virtual ~ComponentGrid() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void recalculate(VulkanDevice& vulkan, const VoxelShapeCache& voxelShapeCache);
    void render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline);
    void update();

    [[nodiscard]] const std::vector<Primitive>& getPrimitives() const {
        return primitives;
    }

private:
    void debugIterate(Grid::Iterator iterator);

    ComponentDebug* debug{nullptr};
    std::vector<Primitive> primitives;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
