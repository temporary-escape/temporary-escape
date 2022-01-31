#pragma once

#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentPlanet : public Component {
public:
    ComponentPlanet() = default;
    explicit ComponentPlanet(Object& object, uint64_t seed) : Component(object), seed(seed) {
    }
    virtual ~ComponentPlanet() = default;

    uint64_t getSeed() const {
        return seed;
    }

private:
    uint64_t seed;

public:
    MSGPACK_DEFINE_ARRAY(seed);
};
} // namespace Engine
