#pragma once

#include "../Vulkan/VulkanRenderer.hpp"

namespace Engine {
class ENGINE_API MaterialTextures {
public:
    class TextureArray {
    public:
        TextureArray(VulkanRenderer& vulkan, const Vector2i& size, TextureCompressionTarget target, size_t count);
        ~TextureArray();

        int addLayer();
        VkFormat getFormat() const {
            return texture.getFormat();
        }

        VulkanTexture& getTexture() {
            return texture;
        }

        const VulkanTexture& getTexture() const {
            return texture;
        }

        void finalize();

    private:
        VulkanRenderer& vulkan;
        Vector2i size;
        size_t count;
        int next;
        VulkanTexture texture;
    };

    using Counts = std::unordered_map<TextureUsage, size_t>;

    explicit MaterialTextures(VulkanRenderer& vulkan, const Vector2i& size, const Counts& counts);

    int addLayer(TextureUsage usage);
    TextureArray& get(TextureUsage usage);
    const TextureArray& get(TextureUsage usage) const;
    void finalize();

private:
    TextureArray diffuse;
    TextureArray emissive;
    TextureArray normal;
    TextureArray metallicRoughness;
    TextureArray ambientOcclusion;
    TextureArray mask;
};
} // namespace Engine
