#pragma once

#include "../Vulkan/VulkanDevice.hpp"

namespace Engine {
class Skybox {
public:
    explicit Skybox(VulkanDevice& vulkan, const Color4& color);

    const VulkanTexture& getTexture() const {
        return texture;
    }

private:
    VulkanTexture texture;
};
} // namespace Engine
