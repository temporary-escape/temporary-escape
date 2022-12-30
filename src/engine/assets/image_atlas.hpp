#pragma once

#include "../config.hpp"
#include "../utils/packer.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include <vector>

namespace Engine {
class ENGINE_API ImageAtlas {
public:
    struct Allocation {
        const VulkanTexture* texture{nullptr};
        Vector2i pos;
        Vector2i size;
        Vector2 uv;
        Vector2 st;
    };

    explicit ImageAtlas(const Config& config, VulkanRenderer& vulkan);

    Allocation add(const Vector2i& size, const void* pixels);

private:
    class Layer {
    public:
        explicit Layer(const Config& config, VulkanRenderer& vulkan);

        std::optional<Vector2i> add(const Vector2i& size, const void* pixels);

        [[nodiscard]] const VulkanTexture& getTexture() const {
            return texture;
        }

    private:
        Packer packer;
        VulkanTexture texture;
    };

    const Config& config;
    VulkanRenderer& vulkan;
    std::vector<std::unique_ptr<Layer>> layers;
};
} // namespace Engine
