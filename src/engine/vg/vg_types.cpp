#include "vg_types.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

VkFormat Engine::toVkFormat(const ImageImporter::PixelType pixelType) {
    switch (pixelType) {
    case ImageImporter::PixelType::Rgb16u: {
        return VK_FORMAT_R16G16B16_UNORM;
    }
    case ImageImporter::PixelType::Rgba16u: {
        return VK_FORMAT_R16G16B16_UNORM;
    }
    case ImageImporter::PixelType::Rgb8u: {
        return VK_FORMAT_R8G8B8_UNORM;
    }
    case ImageImporter::PixelType::Rgba8u: {
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
    }
}

VkDeviceSize Engine::getFormatDataSize(const VkFormat format, const VkExtent3D& extent) {
    switch (format) {
    case VK_FORMAT_R8_UNORM: {
        return extent.width * extent.height * extent.depth;
    }
    case VK_FORMAT_R8G8_UNORM: {
        return extent.width * extent.height * extent.depth * 2;
    }
    case VK_FORMAT_R8G8B8_UNORM: {
        return extent.width * extent.height * extent.depth * 3;
    }
    case VK_FORMAT_R8G8B8A8_UNORM: {
        return extent.width * extent.height * extent.depth * 4;
    }
    case VK_FORMAT_R16_UNORM: {
        return extent.width * extent.height * extent.depth * 2;
    }
    case VK_FORMAT_R16G16_UNORM: {
        return extent.width * extent.height * extent.depth * 4;
    }
    case VK_FORMAT_R16G16B16_UNORM: {
        return extent.width * extent.height * extent.depth * 6;
    }
    case VK_FORMAT_R16G16B16A16_UNORM: {
        return extent.width * extent.height * extent.depth * 8;
    }
    case VK_FORMAT_R32_SFLOAT: {
        return extent.width * extent.height * extent.depth * 4;
    }
    case VK_FORMAT_R32G32_SFLOAT: {
        return extent.width * extent.height * extent.depth * 8;
    }
    case VK_FORMAT_R32G32B32_SFLOAT: {
        return extent.width * extent.height * extent.depth * 12;
    }
    case VK_FORMAT_R32G32B32A32_SFLOAT: {
        return extent.width * extent.height * extent.depth * 16;
    }
    default: {
        EXCEPTION("Unable to calculate texture size, unknown format");
    }
    }
}
