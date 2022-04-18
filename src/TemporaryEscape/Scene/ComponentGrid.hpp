#pragma once

#include "../Assets/Primitive.hpp"
#include "Component.hpp"
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
    explicit ComponentGrid(Object& object) : Component(object) {
    }
    virtual ~ComponentGrid() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void recalculate(Grid::Builder& gridBuilder);
    void update();

    const std::vector<Primitive>& getPrimitives() const {
        return primitives;
    }

private:
    std::vector<Primitive> primitives;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
