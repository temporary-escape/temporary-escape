#pragma once

#include "../Vulkan/VulkanRenderer.hpp"

namespace Engine {
class ENGINE_API SkyboxTextures {
public:
    SkyboxTextures() = default;
    explicit SkyboxTextures(VulkanRenderer& vulkan, const Color4& color);

    VulkanTexture& getTexture() {
        return texture;
    }

    [[nodiscard]] const VulkanTexture& getTexture() const {
        return texture;
    }

    VulkanTexture& getPrefilter() {
        return prefilter;
    }

    [[nodiscard]] const VulkanTexture& getPrefilter() const {
        return prefilter;
    }

    VulkanTexture& getIrradiance() {
        return irradiance;
    }

    [[nodiscard]] const VulkanTexture& getIrradiance() const {
        return irradiance;
    }

    void dispose(VulkanRenderer& vulkan) {
        vulkan.dispose(std::move(texture));
        vulkan.dispose(std::move(prefilter));
        vulkan.dispose(std::move(irradiance));
        texture = VulkanTexture{};
        prefilter = VulkanTexture{};
        irradiance = VulkanTexture{};
    }

private:
    VulkanTexture texture;
    VulkanTexture prefilter;
    VulkanTexture irradiance;
};
} // namespace Engine
