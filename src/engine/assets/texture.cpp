#include "texture.hpp"
#include "../utils/png_importer.hpp"
#include "registry.hpp"

#define CMP "Texture"

using namespace Engine;

void Texture::Options::apply(VulkanTexture::CreateInfo& textureInfo) {
    if (filtering) {
        if (filtering->minification == Filtering::Linear) {
            textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
        } else if (filtering->minification == Filtering::Nearest) {
            textureInfo.sampler.minFilter = VK_FILTER_NEAREST;
        }

        if (filtering->magnification == Filtering::Linear) {
            textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
        } else if (filtering->magnification == Filtering::Nearest) {
            textureInfo.sampler.magFilter = VK_FILTER_NEAREST;
        }
    }

    if (wrapping) {
        if (wrapping->vertical == Wrapping::Repeat) {
            textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        } else if (wrapping->horizontal == Wrapping::ClampToEdge) {
            textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        if (wrapping->horizontal == Wrapping::Repeat) {
            textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        } else if (wrapping->horizontal == Wrapping::ClampToEdge) {
            textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }
}

Texture::Texture(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Texture::load(Registry& registry, VulkanRenderer& vulkan) {
    (void)registry;

    Options options{};
    const auto optionsPath = path.parent_path() / (path.stem().string() + std::string(".yml"));

    if (Fs::exists(optionsPath)) {
        try {
            options.fromYaml(optionsPath);
        } catch (...) {
            EXCEPTION_NESTED("Failed to load texture options from: '{}'", optionsPath.string());
        }
    }

    try {
        PngImporter image(path);

        VkExtent3D extent{static_cast<uint32_t>(image.getSize().x), static_cast<uint32_t>(image.getSize().y), 1};

        VulkanTexture::CreateInfo textureInfo{};
        textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
        textureInfo.image.extent = extent;
        textureInfo.image.mipLevels = 1;
        textureInfo.image.arrayLayers = 1;
        textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
        textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
        textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        textureInfo.view.subresourceRange.baseMipLevel = 0;
        textureInfo.view.subresourceRange.levelCount = 1;
        textureInfo.view.subresourceRange.baseArrayLayer = 0;
        textureInfo.view.subresourceRange.layerCount = 1;

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

        options.apply(textureInfo);

        auto hasAlpha = false;

        switch (image.getPixelType()) {
        case ImageImporter::PixelType::Rgba8u: {
            textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
            hasAlpha = true;
            break;
        }
        case ImageImporter::PixelType::Rgba16u: {
            textureInfo.image.format = VK_FORMAT_R16G16B16A16_UNORM;
            hasAlpha = true;
            break;
        }
        default: {
            EXCEPTION("Unsupported pixel format");
        }
        }

        textureInfo.view.format = textureInfo.image.format;

        if (vulkan.canBeMipMapped(textureInfo.view.format) && hasAlpha) {
            textureInfo.image.mipLevels = getMipMapLevels(image.getSize());
            textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);
            textureInfo.image.usage =
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        texture = vulkan.createTexture(textureInfo);

        vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vulkan.copyDataToImage(texture, 0, {0, 0}, 0, image.getSize(), image.getData());

        if (textureInfo.image.mipLevels > 1) {
            vulkan.generateMipMaps(texture);
        } else {
            vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

TexturePtr Texture::from(const std::string& name) {
    return Registry::getInstance().getTextures().find(name);
}
