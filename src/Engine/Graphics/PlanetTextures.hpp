#pragma once

#include "../Vulkan/VulkanRenderer.hpp"

namespace Engine {
class ENGINE_API PlanetTextures {
public:
    PlanetTextures() = default;

    VulkanTexture& getColor() {
        return color;
    }

    [[nodiscard]] const VulkanTexture& getColor() const {
        return color;
    }

    VulkanTexture& getMetallicRoughness() {
        return metallicRoughness;
    }

    [[nodiscard]] const VulkanTexture& getMetallicRoughness() const {
        return metallicRoughness;
    }

    VulkanTexture& getNormal() {
        return normal;
    }

    [[nodiscard]] const VulkanTexture& getNormal() const {
        return normal;
    }

    void dispose(VulkanRenderer& vulkan) {
        vulkan.dispose(std::move(color));
        vulkan.dispose(std::move(metallicRoughness));
        vulkan.dispose(std::move(normal));
        color = VulkanTexture{};
        metallicRoughness = VulkanTexture{};
        normal = VulkanTexture{};
    }

private:
    VulkanTexture color;
    VulkanTexture metallicRoughness;
    VulkanTexture normal;
};
} // namespace Engine
