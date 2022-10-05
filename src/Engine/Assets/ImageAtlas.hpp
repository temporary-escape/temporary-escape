#pragma once

#include "../Config.hpp"
#include "../Utils/Packer.hpp"
#include "../Vulkan/VulkanDevice.hpp"
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

    explicit ImageAtlas(const Config& config, VulkanDevice& vulkan);

    Allocation add(const Vector2i& size, const void* pixels);

private:
    class Layer {
    public:
        explicit Layer(const Config& config, VulkanDevice& vulkan);

        std::optional<Vector2i> add(const Vector2i& size, const void* pixels);

        [[nodiscard]] const VulkanTexture& getTexture() const {
            return texture;
        }

    private:
        Packer packer;
        VulkanTexture texture;
    };

    const Config& config;
    VulkanDevice& vulkan;
    std::vector<std::unique_ptr<Layer>> layers;
};
} // namespace Engine
