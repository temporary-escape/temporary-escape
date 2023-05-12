#pragma once

#include "../graphics/skybox_textures.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentSkybox : public Component {
public:
    ComponentSkybox() = default;
    explicit ComponentSkybox(uint64_t seed) : seed{seed} {
    }
    virtual ~ComponentSkybox() = default; // NOLINT(modernize-use-override)
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

private:
    SkyboxTextures textures{};
    uint64_t seed{0};
    bool generated{false};
};
} // namespace Engine
