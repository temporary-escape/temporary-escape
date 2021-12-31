#pragma once

#include "Component.hpp"

namespace Scissio {
class SCISSIO_API ComponentSkybox : public Component {
public:
    ComponentSkybox() = default;
    explicit ComponentSkybox(Object& object, uint64_t seed) : Component(object), seed(seed) {
    }
    virtual ~ComponentSkybox() = default;

    uint64_t getSeed() const {
        return seed;
    }

private:
    uint64_t seed;

public:
    MSGPACK_DEFINE_ARRAY(seed);
};
} // namespace Scissio
