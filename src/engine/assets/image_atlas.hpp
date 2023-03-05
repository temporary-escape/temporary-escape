#pragma once

#include "../config.hpp"
#include "../utils/packer.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include <functional>
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

    class Layer {
    public:
        explicit Layer(const Config& config, VulkanRenderer& vulkan);

        std::optional<Vector2i> add(const Vector2i& size, const void* pixels);
        std::optional<Vector2i> add(const Vector2i& size, const VulkanTexture& source);
        void finalize();

        [[nodiscard]] const VulkanTexture& getTexture() const {
            return texture;
        }

    private:
        VulkanRenderer& vulkan;
        Packer packer;
        VulkanTexture texture;
    };

    explicit ImageAtlas(const Config& config, VulkanRenderer& vulkan);
    ImageAtlas(const ImageAtlas& other) = delete;
    ImageAtlas(ImageAtlas&& other) = default;
    ImageAtlas& operator=(const ImageAtlas& other) = delete;
    ImageAtlas& operator=(ImageAtlas&& other) = default;

    Allocation add(const Vector2i& size, const void* pixels);
    Allocation add(const Vector2i& size, const VulkanTexture& source);
    void finalize();

private:
    using LayerAddFunc = std::function<std::optional<Vector2i>(Layer&)>;
    Allocation add(const Vector2i& size, const LayerAddFunc& provider);

    const Config* config;
    VulkanRenderer* vulkan;
    std::list<std::unique_ptr<Layer>> layers;
};
} // namespace Engine
