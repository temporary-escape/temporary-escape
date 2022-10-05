#pragma once

#include "Texture.hpp"

namespace Engine {
struct Material {
    TexturePtr baseColorTexture;
    TexturePtr emissiveTexture;
    TexturePtr normalTexture;
    TexturePtr ambientOcclusionTexture;
    TexturePtr metallicRoughnessTexture;

    struct Uniform {
        Vector4 baseColorFactor;
        Vector4 emissiveFactor;
        Vector4 normalFactor;
        Vector4 ambientOcclusionFactor;
        Vector4 metallicRoughnessFactor;
    } uniform;

    VulkanBuffer ubo;
};
} // namespace Engine
