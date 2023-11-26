#include "SkyboxTextures.hpp"

using namespace Engine;

SkyboxTextures::SkyboxTextures(VulkanRenderer& vulkan, const Color4& color) {
    static const Vector2i size = {8, 8};
    std::unique_ptr<uint8_t[]> pixels(new uint8_t[size.x * size.y * 4]);

    for (size_t i = 0; i < size.x * size.y * 4; i += 4) {
        pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
        pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
        pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
        pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
    }

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 6;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    textureInfo.image.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 6;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
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
    irradiance = vulkan.createTexture(textureInfo);
    prefilter = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.transitionImageLayout(irradiance, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.transitionImageLayout(prefilter, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    for (auto i = 0; i < 6; i++) {
        vulkan.copyDataToImage(texture, 0, {0, 0}, i, size, pixels.get());
        vulkan.copyDataToImage(irradiance, 0, {0, 0}, i, size, pixels.get());
        vulkan.copyDataToImage(prefilter, 0, {0, 0}, i, size, pixels.get());
    }

    vulkan.transitionImageLayout(
        texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vulkan.transitionImageLayout(
        irradiance, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vulkan.transitionImageLayout(
        prefilter, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
