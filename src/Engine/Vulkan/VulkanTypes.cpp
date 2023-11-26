#include "VulkanTypes.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

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
    case VK_FORMAT_R8_SRGB: {
        return extent.width * extent.height * extent.depth;
    }
    case VK_FORMAT_R8G8_SRGB: {
        return extent.width * extent.height * extent.depth * 2;
    }
    case VK_FORMAT_R8G8B8_SRGB: {
        return extent.width * extent.height * extent.depth * 3;
    }
    case VK_FORMAT_R8G8B8A8_SRGB: {
        return extent.width * extent.height * extent.depth * 4;
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
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: {
        return (extent.width * extent.height * extent.depth) * 4;
    }
    default: {
        EXCEPTION("Unable to calculate texture size, unknown format");
    }
    }
}

bool Engine::isDepthFormat(const VkFormat format) {
    switch (format) {
    case VkFormat::VK_FORMAT_D24_UNORM_S8_UINT:
    case VkFormat::VK_FORMAT_D16_UNORM:
    case VkFormat::VK_FORMAT_D16_UNORM_S8_UINT:
    case VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32:
    case VkFormat::VK_FORMAT_D32_SFLOAT:
    case VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT: {
        return true;
    }
    default: {
        return false;
    }
    }
}
