#pragma once

#include "../assets/planet_type.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPlanet : public Component {
public:
    ComponentPlanet() = default;
    explicit ComponentPlanet(PlanetTypePtr planetType) : planetType{std::move(planetType)} {
    }
    virtual ~ComponentPlanet() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPlanet);

    [[nodiscard]] const PlanetTypePtr& getPlanetType() const {
        return planetType;
    }

private:
    PlanetTypePtr planetType;
};
} // namespace Engine
