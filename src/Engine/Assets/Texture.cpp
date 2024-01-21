#include "Texture.hpp"
#include "../File/Ktx2FileReader.hpp"
#include "../File/PngFileReader.hpp"
#include "../Utils/StringUtils.hpp"
#include "AssetsManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void Texture::Options::apply(VulkanTexture::CreateInfo& textureInfo) const {
    if (filtering.minification == Filtering::Linear) {
        textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    } else if (filtering.minification == Filtering::Nearest) {
        textureInfo.sampler.minFilter = VK_FILTER_NEAREST;
    }

    if (filtering.magnification == Filtering::Linear) {
        textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    } else if (filtering.magnification == Filtering::Nearest) {
        textureInfo.sampler.magFilter = VK_FILTER_NEAREST;
    }

    if (wrapping.vertical == Wrapping::Repeat) {
        textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    } else if (wrapping.horizontal == Wrapping::ClampToEdge) {
        textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }

    if (wrapping.horizontal == Wrapping::Repeat) {
        textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    } else if (wrapping.horizontal == Wrapping::ClampToEdge) {
        textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }

    if (type == Type::Texture1D) {
        textureInfo.image.imageType = VK_IMAGE_TYPE_1D;
        textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_1D;
    } else if (type == Type::Texture2D) {
        textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
        textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }

    if (srgb) {
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
    const auto optionsPath = path.parent_path() / (path.stem().string() + std::string(".xml"));

    if (Fs::exists(optionsPath)) {
        try {
            logger.info("Loading texture options from: '{}'", optionsPath);
            Xml::fromFile(optionsPath, options);
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

    try {
        loadKtx2(assetsManager, *vulkan);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

static TextureCompressionTarget getCompressionTarget(const std::string& filename) {
    if (endsWith(filename, "_diff")) {
        return TextureCompressionTarget::RGBA;

    } else if (endsWith(filename, "_meta") || endsWith(filename, "_emis")) {
        return TextureCompressionTarget::RGB;

    } else if (endsWith(filename, "_norm")) {
        return TextureCompressionTarget::RG;

    } else if (endsWith(filename, "_ao") || endsWith(filename, "_mask")) {
        return TextureCompressionTarget::R;

    } else {
        return TextureCompressionTarget::RGBA;
    }
}

void Texture::loadKtx2(AssetsManager& assetsManager, VulkanRenderer& vulkan) {
    Ktx2FileReader image{path};

    if (image.needsTranscoding()) {
        logger.debug("Transcoding texture: {}", getPath());
        const auto compressionTarget = getCompressionTarget(getPath().stem().string());
        image.transcode(vulkan.getCompressionType(), compressionTarget);
    }

    image.readData();

    if (usage == TextureUsage::Any) {
        const auto options = loadOptions(path);
        loadAsAny(options, vulkan, image);
    } else {
        loadAsLayer(assetsManager, vulkan, image);
    }
}

void Texture::loadAsAny(const Texture::Options& options, VulkanRenderer& vulkan, Ktx2FileReader& image) {
    VkExtent3D extent{static_cast<uint32_t>(image.getSize().x),
                      static_cast<uint32_t>(image.getSize().y),
                      static_cast<uint32_t>(image.getSize().z)};

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = image.getFormat();
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = extent;
    textureInfo.image.mipLevels = image.getMipMapsCount();
    textureInfo.image.arrayLayers = 1;
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
    textureInfo.view.subresourceRange.layerCount = 1;

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

void Texture::loadAsLayer(AssetsManager& assetsManager, VulkanRenderer& vulkan, Ktx2FileReader& image) {
    layer = assetsManager.getMaterialTextures().addLayer(usage);
    auto& array = assetsManager.getMaterialTextures().get(usage);
    auto& tex = array.getTexture();

    Vector3i expectedSize = {
        tex.getExtent().width,
        tex.getExtent().height,
        1,
    };
    if (expectedSize != image.getSize()) {
        EXCEPTION("Texture has bad size: {} expected: {}", image.getSize(), expectedSize);
    }
    if (tex.getFormat() != image.getFormat()) {
        EXCEPTION("Texture has bad format: {} expected: {}", image.getFormat(), tex.getFormat());
    }
    if (tex.getMipMaps() != image.getMipMapsCount()) {
        EXCEPTION("Texture has incorrect mipmaps: {} expected: {}", image.getMipMapsCount(), tex.getMipMaps());
    }

    for (auto i = 0; i < image.getMipMapsCount(); i++) {
        const auto& chunk = image.getData(i, 0);
        vulkan.copyDataToImage(tex, i, {0, 0}, layer, chunk.size, chunk.pixels.data(), chunk.pixels.size());
    }
}

void Texture::setUsage(const TextureUsage value) {
    if (usage != TextureUsage::Any && value != usage) {
        EXCEPTION("Can not use texture: '{}' as: {}, error: texture is set to: {}",
                  getName(),
                  static_cast<int>(value),
                  static_cast<int>(usage));
    }
    usage = value;
}

TexturePtr Texture::from(const std::string& name) {
    return AssetsManager::getInstance().getTextures().find(name);
}
