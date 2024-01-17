#pragma once

#include "../../Assets/PlanetType.hpp"
#include "../../Graphics/PlanetTextures.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentPlanet : public Component {
public:
    ComponentPlanet() = default;
    explicit ComponentPlanet(EntityId entity, PlanetTypePtr planetType, const uint64_t seed);
    COMPONENT_DEFAULTS(ComponentPlanet);

    [[nodiscard]] const PlanetTypePtr& getPlanetType() const {
        return planetType;
    }

    [[nodiscard]] uint64_t getSeed() const {
        return seed;
    }

    [[nodiscard]] bool isGenerated() const {
        return generated || !highRes;
    }

    [[nodiscard]] bool isHighRes() const {
        return highRes;
    }

    [[nodiscard]] bool isBackground() const {
        return background;
    }

    void setHighRes(const bool value) {
        highRes = value;
    }

    void setBackground(const bool value) {
        background = value;
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
    bool highRes{false};
    bool background{true};
};
} // namespace Engine
