#pragma once

#include "../vulkan/vulkan_renderer.hpp"

namespace Engine {
class ENGINE_API Skybox {
public:
    Skybox() = default;
    explicit Skybox(VulkanRenderer& vulkan, const Color4& color);

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

private:
    VulkanTexture texture;
    VulkanTexture prefilter;
    VulkanTexture irradiance;
};
} // namespace Engine
