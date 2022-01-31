#pragma once

#include "AssetTexture.hpp"

namespace Engine {
struct Material {
    struct Uniform {
        Vector4 baseColorFactor{1.0f};
        Vector4 emissiveFactor{1.0f};
        Vector4 metallicRoughnessFactor{1.0f};
    } uniform;

    AssetTexturePtr baseColorTexture{nullptr};
    AssetTexturePtr emissiveTexture{nullptr};
    AssetTexturePtr normalTexture{nullptr};
    AssetTexturePtr ambientOcclusionTexture{nullptr};
    AssetTexturePtr metallicRoughnessTexture{nullptr};
};
} // namespace Engine
