#include "texture.hpp"
#include "../file/ktx2_file_reader.hpp"
#include "../file/png_file_reader.hpp"
#include "../server/lua.hpp"
#include "../utils/string_utils.hpp"
#include "assets_manager.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void Texture::Options::apply(VulkanTexture::CreateInfo& textureInfo) const {
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

    if (type) {
        if (type == Type::Texture1D) {
            textureInfo.image.imageType = VK_IMAGE_TYPE_1D;
            textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_1D;
        } else if (type == Type::Texture2D) {
            textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
            textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        }
    }

    if (srgb && *srgb) {
        if (textureInfo.image.format == VK_FORMAT_R8G8B8A8_UNORM) {
            textureInfo.image.format = VK_FORMAT_R8G8B8A8_SRGB;
            textureInfo.view.format = VK_FORMAT_R8G8B8A8_SRGB;
        } else if (textureInfo.image.format == VK_FORMAT_R8G8B8_UNORM) {
            textureInfo.image.format = VK_FORMAT_R8G8B8_SRGB;
            textureInfo.view.format = VK_FORMAT_R8G8B8_SRGB;
        }
    }
}

Texture::Options Texture::loadOptions(const Path& path) {
    Options options{};
    const auto optionsPath = path.parent_path() / (path.stem().string() + std::string(".yml"));

    if (Fs::exists(optionsPath)) {
        try {
            options.fromYaml(optionsPath);
        } catch (...) {
            EXCEPTION_NESTED("Failed to load texture options from: '{}'", optionsPath.string());
        }
    }

    return options;
}

Texture::Texture(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Texture::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)assetsManager;
    (void)audio;

    // Do not load unless Vulkan is present (client mode)
    if (!vulkan) {
        return;
    }

    const auto options = loadOptions(path);

    try {
        if (path.extension().string() == ".png") {
            loadPng(options, *vulkan);
        } else if (path.extension().string() == ".ktx2") {
            loadKtx2(options, *vulkan);
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

void Texture::loadPng(const Options& options, VulkanRenderer& vulkan) {
    PngFileReader image{path};

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

    if (const auto anisotropy = vulkan.getAnisotropy()) {
        textureInfo.sampler.anisotropyEnable = anisotropy.has_value() ? VK_TRUE : VK_FALSE;
        textureInfo.sampler.maxAnisotropy = anisotropy.has_value() ? *anisotropy : 1.0f;
    } else {
        textureInfo.sampler.anisotropyEnable = VK_FALSE;
        textureInfo.sampler.maxAnisotropy = 1.0f;
    }

    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    options.apply(textureInfo);

    switch (image.getFormat()) {
    case VK_FORMAT_R8G8B8A8_UNORM: {
        if (options.srgb && *options.srgb) {
            textureInfo.image.format = VK_FORMAT_R8G8B8A8_SRGB;
        } else {
            textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
        }
        break;
    }
    case VK_FORMAT_R16G16B16A16_UNORM: {
        textureInfo.image.format = VK_FORMAT_R16G16B16A16_UNORM;
        break;
    }
    default: {
        EXCEPTION("Unsupported pixel format");
    }
    }

    textureInfo.view.format = textureInfo.image.format;

    if (vulkan.canBeMipMapped(textureInfo.view.format)) {
        textureInfo.image.mipLevels = getMipMapLevels(image.getSize());
        textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);
        textureInfo.view.subresourceRange.levelCount = textureInfo.image.mipLevels;
        textureInfo.image.usage =
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    } else {
        logger.warn("Texture: '{}' can not be mip-mapped", getPath());
    }

    texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.copyDataToImage(texture, 0, {0, 0}, 0, image.getSize(), image.getData());

    if (textureInfo.image.mipLevels > 1) {
        logger.debug("Texture: '{}' generating {} mip-maps", getPath(), textureInfo.image.mipLevels);
        vulkan.generateMipMaps(texture);
    } else {
        vulkan.transitionImageLayout(
            texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

static Ktx2CompressionTarget getCompressionTarget(const std::string& filename) {
    if (endsWith(filename, "_diff")) {
        return Ktx2CompressionTarget::RGBA;

    } else if (endsWith(filename, "_meta") || endsWith(filename, "_emis")) {
        return Ktx2CompressionTarget::RGB;

    } else if (endsWith(filename, "_norm")) {
        return Ktx2CompressionTarget::RG;

    } else if (endsWith(filename, "_ao") || endsWith(filename, "_mask")) {
        return Ktx2CompressionTarget::R;

    } else {
        return Ktx2CompressionTarget::RGBA;
    }
}

void Texture::loadKtx2(const Texture::Options& options, VulkanRenderer& vulkan) {
    Ktx2FileReader image{path};

    if (image.needsTranscoding()) {
        logger.debug("Transcoding texture: {}", getPath());
        const auto compressionTarget = getCompressionTarget(getPath().stem().string());
        image.transcode(vulkan.getCompressionType(), compressionTarget);
    }

    image.readData();

    VkExtent3D extent{static_cast<uint32_t>(image.getSize().x),
                      static_cast<uint32_t>(image.getSize().y),
                      static_cast<uint32_t>(image.getSize().z)};

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = image.getFormat();
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = extent;
    textureInfo.image.mipLevels = image.getMipMapsCount();
    textureInfo.image.arrayLayers = image.getLayersCount();
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = image.getFormat();
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = image.getMipMapsCount();
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = image.getLayersCount();

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;

    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    if (vulkan.getPhysicalDeviceFeatures().samplerAnisotropy) {
        textureInfo.sampler.anisotropyEnable = VK_TRUE;
        textureInfo.sampler.maxAnisotropy =
            std::max(4.0f, vulkan.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy);
    } else {
        textureInfo.sampler.anisotropyEnable = VK_FALSE;
        textureInfo.sampler.maxAnisotropy = 1.0f;
    }

    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    options.apply(textureInfo);

    texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    for (auto i = 0; i < image.getMipMapsCount(); i++) {
        const auto& chunk = image.getData(i, 0);
        vulkan.copyDataToImage(texture, i, {0, 0}, 0, chunk.size, chunk.pixels.data(), chunk.pixels.size());
    }

    vulkan.transitionImageLayout(
        texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

TexturePtr Texture::from(const std::string& name) {
    return AssetsManager::getInstance().getTextures().find(name);
}

void Texture::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Texture>("Texture");
    cls["name"] = sol::property(&Texture::getName);
}
