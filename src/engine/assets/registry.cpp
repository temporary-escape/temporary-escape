#include "registry.hpp"

#define CMP "Registry"

using namespace Engine;

Registry* Registry::instance{nullptr};

Registry::Registry(const Config& config) : config{config} {
    instance = this;
    voxelShapeCache = std::make_unique<VoxelShapeCache>(this->config);
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

bool Registry::isReady() {
    return loadQueue.empty();
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

    /*auto desc = VulkanTexture::Descriptor{};
    desc.size = Vector2i{4, 4};
    desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
    desc.levels = 1;
    desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    desc.usage =
        VulkanTexture::Usage::VK_IMAGE_USAGE_SAMPLED_BIT | VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    asset->getVulkanTexture() = vulkan.createTexture(desc);
    asset->getVulkanTexture().subData(0, {0, 0}, {4, 4}, pixels.get());*/

    return asset;
}
