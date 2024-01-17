#pragma once

#include "../../Graphics/SkyboxTextures.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentSkybox : public Component {
public:
    ComponentSkybox() = default;
    explicit ComponentSkybox(EntityId entity, uint64_t seed);
    COMPONENT_DEFAULTS(ComponentSkybox);

    [[nodiscard]] uint64_t getSeed() const {
        return seed;
    }

    [[nodiscard]] bool isGenerated() const {
        return generated;
    }

    void setTextures(VulkanRenderer& vulkan, SkyboxTextures value) {
        generated = true;
        textures.dispose(vulkan);
        textures = std::move(value);
    }

    const SkyboxTextures& getTextures() const {
        return textures;
    }

    void setStars(const bool value) {
        stars = value;
    }

    bool getStars() const {
        return stars;
    }

private:
    SkyboxTextures textures{};
    uint64_t seed{0};
    bool generated{false};
    bool stars{false};
};
} // namespace Engine
