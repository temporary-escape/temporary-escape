#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPlanet : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentPlanet() = default;
    explicit ComponentPlanet(Object& object, uint64_t seed) : Component(object), seed(seed) {
    }
    virtual ~ComponentPlanet() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    uint64_t getSeed() const {
        return seed;
    }

private:
    uint64_t seed;

public:
    MSGPACK_DEFINE_ARRAY(seed);
};
} // namespace Engine
