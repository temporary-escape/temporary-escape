#pragma once

#include "../vulkan/vulkan_texture.hpp"

namespace Engine {
class ENGINE_API VulkanRenderer;

class ENGINE_API VoxelPalette {
public:
    VoxelPalette(const Config& config, VulkanRenderer& vulkan);

    const VulkanTexture& getTexture() const {
        return texture;
    }

    const Color4& getColor(size_t index) const {
        return colors[index];
    }

    size_t getSize() const {
        return colors.size();
    }

private:
    VulkanTexture texture;
    std::array<Color4, 16 * 4> colors;
};
} // namespace Engine
