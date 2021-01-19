#include "PixelType.hpp"

#include <IL/il.h>

using namespace Scissio;

#define GL_COMPRESSED_RED_RGTC1_EXT 0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1_EXT 0x8DBC
#define GL_COMPRESSED_RED_GREEN_RGTC2_EXT 0x8DBD
#define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT 0x8DBE
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

PixelType Scissio::toPixelType(const int bpp, const int ilFlag) {
    switch (ilFlag) {
    case IL_RGB: {
        switch (bpp) {
        case 24: {
            return PixelType::Rgb8u;
        }
        default: {
            break;
        }
        }
    }
    case IL_RGBA: {
        switch (bpp) {
        case 32: {
            return PixelType::Rgba8u;
        }
        default: {
            break;
        }
        }
    }
    default: {
        break;
    }
    }

    return PixelType::None;
}

GLenum Scissio::toTextureFormat(const PixelType type) {
    switch (type) {
    case PixelType::Red8u:
    case PixelType::Red16u:
    case PixelType::Red16f:
    case PixelType::Red32u:
    case PixelType::Red32f: {
        return GL_RED;
    }
    case PixelType::Rg8u:
    case PixelType::Rg16u:
    case PixelType::Rg16f:
    case PixelType::Rg32u:
    case PixelType::Rg32f: {
        return GL_RG;
    }
    case PixelType::Rgb8u:
    case PixelType::Rgb16u:
    case PixelType::Rgb16f:
    case PixelType::Rgb32u:
    case PixelType::Rgb32f: {
        return GL_RGB;
    }
    case PixelType::Rgba8u:
    case PixelType::Rgba16u:
    case PixelType::Rgba16f:
    case PixelType::Rgba32u:
    case PixelType::Rgba32f: {
        return GL_RGBA;
    }
    case PixelType::CompressedRGBS3tcDxt1: {
        return GL_RGB;
    }
    case PixelType::CompressedRGBAS3tcDxt5:
    case PixelType::CompressedRGBAS3tcDxt3:
    case PixelType::CompressedRGBAS3tcDxt1: {
        return GL_RGBA;
    }
    case PixelType::CompressedRGRgtc2Signed:
    case PixelType::CompressedRGRgtc2: {
        return GL_RG;
    }
    case PixelType::CompressedRedRgtc1Signed:
    case PixelType::CompressedRedRgtc1: {
        return GL_RED;
    }
    case PixelType::Depth24Stencil8: {
        return GL_DEPTH_STENCIL;
    }
    default: {
        return GL_NONE;
    }
    }
}

GLenum Scissio::toTextureInternalFormat(const PixelType type) {
    switch (type) {
    case PixelType::Red8u: {
        return GL_R8;
    }
    case PixelType::Rg8u: {
        return GL_RG8;
    }
    case PixelType::Rgb8u: {
        return GL_RGB8;
    }
    case PixelType::Rgba8u: {
        return GL_RGBA8;
    }
    case PixelType::Red16u: {
        return GL_R16;
    }
    case PixelType::Rg16u: {
        return GL_RG16;
    }
    case PixelType::Rgb16u: {
        return GL_RGB16;
    }
    case PixelType::Rgba16u: {
        return GL_RGBA16;
    }
    case PixelType::Red16f: {
        return GL_R16F;
    }
    case PixelType::Rg16f: {
        return GL_RG16F;
    }
    case PixelType::Rgb16f: {
        return GL_RGB16F;
    }
    case PixelType::Rgba16f: {
        return GL_RGBA16F;
    }
    case PixelType::Red32f: {
        return GL_R32F;
    }
    case PixelType::Rg32f: {
        return GL_RG32F;
    }
    case PixelType::Rgb32f: {
        return GL_RGB32F;
    }
    case PixelType::Rgba32f: {
        return GL_RGBA32F;
    }
    case PixelType::Red32u: {
        return GL_R32UI;
    }
    case PixelType::Rg32u: {
        return GL_RG32UI;
    }
    case PixelType::Rgb32u: {
        return GL_RGB32UI;
    }
    case PixelType::Rgba32u: {
        return GL_RGBA32UI;
    }
    case PixelType::CompressedRGBAS3tcDxt5: {
        return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
    case PixelType::CompressedRGBAS3tcDxt3: {
        return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    }
    case PixelType::CompressedRGBAS3tcDxt1: {
        return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    }
    case PixelType::CompressedRGBS3tcDxt1: {
        return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    }
    case PixelType::CompressedRGRgtc2: {
        return GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
    }
    case PixelType::CompressedRedRgtc1: {
        return GL_COMPRESSED_RED_RGTC1_EXT;
    }
    case PixelType::CompressedRGRgtc2Signed: {
        return GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT;
    }
    case PixelType::CompressedRedRgtc1Signed: {
        return GL_COMPRESSED_SIGNED_RED_RGTC1_EXT;
    }
    case PixelType::Depth24Stencil8: {
        return GL_DEPTH24_STENCIL8;
    }
    default: {
        return GL_NONE;
    }
    }
}

GLenum Scissio::toTextureType(const PixelType type) {
    switch (type) {
    case PixelType::Red8u:
    case PixelType::Rg8u:
    case PixelType::Rgb8u:
    case PixelType::Rgba8u: {
        return GL_UNSIGNED_BYTE;
    }
    case PixelType::Red16u:
    case PixelType::Rg16u:
    case PixelType::Rgb16u:
    case PixelType::Rgba16u: {
        return GL_UNSIGNED_SHORT;
    }
    case PixelType::Red16f:
    case PixelType::Rg16f:
    case PixelType::Rgb16f:
    case PixelType::Rgba16f: {
        return GL_HALF_FLOAT;
    }
    case PixelType::Red32u:
    case PixelType::Rg32u:
    case PixelType::Rgb32u:
    case PixelType::Rgba32u: {
        return GL_UNSIGNED_INT;
    }
    case PixelType::Red32f:
    case PixelType::Rg32f:
    case PixelType::Rgb32f:
    case PixelType::Rgba32f: {
        return GL_FLOAT;
    }
    case PixelType::Depth24Stencil8: {
        return GL_UNSIGNED_INT_24_8;
    }
    default: {
        return GL_NONE;
    }
    }
}
