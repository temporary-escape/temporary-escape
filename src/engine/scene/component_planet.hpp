#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPlanet : public Component {
public:
    ComponentPlanet() = default;
    explicit ComponentPlanet(uint64_t seed) : seed{seed} {
    }
    virtual ~ComponentPlanet() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPlanet);

    [[nodiscard]] uint64_t getSeed() const {
        return seed;
    }

private:
    uint64_t seed;
};
} // namespace Engine
