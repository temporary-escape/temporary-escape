#include "registry.hpp"

#define CMP "Registry"

using namespace Engine;

Registry* Registry::instance{nullptr};

Registry::Registry(const Config& config) : config{config} {
    instance = this;
    findAssets();
}

void Registry::init(VulkanRenderer& vulkan) {
    try {
        if (!atlas) {
            atlas = std::make_unique<ImageAtlas>(config, vulkan);
        }

        if (!defaultTextures) {
            defaultTextures = std::make_unique<DefaultTextures>();
            defaultTextures->baseColor =
                createTextureOfColor(vulkan, Color4{1.0f, 0.0f, 1.0f, 1.0f}, "default_baseColor");
            defaultTextures->ambient = createTextureOfColor(vulkan, Color4{1.0f, 0.0f, 0.0f, 1.0f}, "default_ambient");
            defaultTextures->emissive =
                createTextureOfColor(vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}, "default_emissive");
            defaultTextures->metallicRoughness =
                createTextureOfColor(vulkan, Color4{0.0f, 0.5f, 0.5f, 1.0f}, "default_metallicRoughness");
            defaultTextures->normal = createTextureOfColor(vulkan, Color4{0.5f, 0.5f, 1.0f, 1.0f}, "default_normal");
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to init registry");
    }
}

void Registry::findAssets() {
    try {
        const std::vector<Path> paths = {config.assetsPath / "base"};

        for (const auto& path : paths) {
            addManifest(path);
        }

        for (const auto& path : paths) {
            init(images, config.assetsPath / "base" / "images", {".png"});
        }

        for (const auto& path : paths) {
            init(textures, config.assetsPath / "base" / "textures", {".png"});
        }

        for (const auto& path : paths) {
            init(models, config.assetsPath / "base" / "models", {".gltf"});
        }

        for (const auto& path : paths) {
            init(blocks, config.assetsPath / "base" / "blocks", {".yml", ".yaml"});
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to initialize registry");
    }
}

void Registry::addManifest(const Path& path) {
    try {
        ModManifest manifest{};
        manifest.fromYaml(path / "manifest.yml");
        manifests.push_back(manifest);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load manifest for mod package: \'{}\'", path);
    }
}

Registry& Registry::getInstance() {
    if (!instance) {
        EXCEPTION("No instance initialized of registry")
    }
    return *instance;
}

TexturePtr Registry::createTextureOfColor(VulkanRenderer& vulkan, const Color4& color, const std::string& name) {
    auto asset = std::make_shared<Texture>(name, "");
    textures.insert(name, asset);

    std::unique_ptr<uint8_t[]> pixels(new uint8_t[4 * 4 * 4]);

    for (size_t i = 0; i < 4 * 4 * 4; i += 4) {
        pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
        pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
        pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
        pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
    }

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {4, 4, 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    auto& texture = asset->getVulkanTexture();
    texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.copyDataToImage(texture, 0, {0, 0}, 0, {4, 4}, pixels.get());
    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return asset;
}
