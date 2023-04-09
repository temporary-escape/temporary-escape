#pragma once

#include "../assets/planet_type.hpp"
#include "../graphics/planet_textures.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPlanet : public Component {
public:
    ComponentPlanet() = default;
    explicit ComponentPlanet(PlanetTypePtr planetType, uint64_t seed) : planetType{std::move(planetType)}, seed{seed} {
    }
    virtual ~ComponentPlanet() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPlanet);

    [[nodiscard]] const PlanetTypePtr& getPlanetType() const {
        return planetType;
    }

    [[nodiscard]] uint64_t getSeed() const {
        return seed;
    }

    [[nodiscard]] bool isGenerated() const {
        return generated;
    }

    void setTextures(VulkanRenderer& vulkan, PlanetTextures value) {
        generated = true;
        textures.dispose(vulkan);
        textures = std::move(value);
    }

    const PlanetTextures& getTextures() const {
        return textures;
    }

private:
    PlanetTypePtr planetType;
    PlanetTextures textures{};
    uint64_t seed{0};
    bool generated{false};
};
} // namespace Engine
