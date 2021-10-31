#pragma once

#include "PbrTexture.hpp"

namespace Scissio {
struct Material {
    Vector4 baseColorFactor{1.0f};
    Vector4 emissiveFactor{1.0f};
    Vector4 metallicRoughnessFactor{1.0f};
    PbrTexturePtr baseColorTexture{nullptr};
    PbrTexturePtr emissiveTexture{nullptr};
    PbrTexturePtr normalTexture{nullptr};
    PbrTexturePtr ambientOcclusionTexture{nullptr};
    PbrTexturePtr metallicRoughnessTexture{nullptr};
};
} // namespace Scissio
