#include "voxel_palette.hpp"
#include "../math/utils.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/png_importer.hpp"
#include "../vulkan/vulkan_renderer.hpp"

using namespace Engine;

VoxelPalette::VoxelPalette(const Config& config, VulkanRenderer& vulkan) {
    PngImporter img{config.assetsPath / "palette.png"};

    if (img.getSize().x != colors.size() || img.getSize().y != 1) {
        EXCEPTION("Palette has wrong size");
    }

    if (img.getPixelType() != ImageImporter::PixelType::Rgba8u) {
        EXCEPTION("Palette has wrong pixel type, expected RGBA8");
    }

    const auto* data = img.getData();
    for (size_t i = 0; i < colors.size(); i++) {
        const auto* src = &reinterpret_cast<const uint8_t*>(data)[i * 4];
        colors[i] = fromRgbBytes(src[0], src[1], src[2], 255);
        colors[i] = glm::pow(colors[i], Vector4{2.2f});
    }

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.imageType = VK_IMAGE_TYPE_1D;
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.extent = {static_cast<uint32_t>(img.getSize().x), static_cast<uint32_t>(img.getSize().y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_1D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_NEAREST;
    textureInfo.sampler.minFilter = VK_FILTER_NEAREST;

    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.copyDataToImage(texture, 0, {0, 0}, 0, img.getSize(), img.getData());
    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
