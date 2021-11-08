#pragma once

#include "../Library.hpp"

#include <glad/glad.h>

namespace Scissio {
enum class PixelType : int {
    None = 0,
    Red8u,
    Rg8u,
    Rgb8u,
    Rgba8u,
    Red16u,
    Rg16u,
    Rgb16u,
    Rgba16u,
    Red32u,
    Rg32u,
    Rgb32u,
    Rgba32u,
    Red16f,
    Rg16f,
    Rgb16f,
    Rgba16f,
    Red32f,
    Rg32f,
    Rgb32f,
    Rgba32f,
    CompressedRGBS3tcDxt1,
    CompressedRGBAS3tcDxt5,
    CompressedRGBAS3tcDxt3,
    CompressedRGBAS3tcDxt1,
    CompressedRGRgtc2,
    CompressedRedRgtc1,
    CompressedRGRgtc2Signed,
    CompressedRedRgtc1Signed,
    Depth24Stencil8,
};

SCISSIO_API GLenum toTextureFormat(PixelType type);
SCISSIO_API GLenum toTextureInternalFormat(PixelType type);
SCISSIO_API GLenum toTextureType(PixelType type);
} // namespace Scissio
