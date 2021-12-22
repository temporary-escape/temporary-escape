#pragma once

#include "Component.hpp"

namespace Scissio {
class SCISSIO_API ComponentSkybox : public Component {
public:
    static constexpr ComponentSkybox Type = 1;

    ComponentSkybox();
    explicit ComponentSkybox(Object& object, uint64_t seed);
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
