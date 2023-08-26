#include "assets_manager.hpp"
#include "../file/ktx2_file_reader.hpp"
#include "../server/lua.hpp"
#include "../vulkan/glsl_compiler.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

AssetsManager* AssetsManager::instance{nullptr};

static void compressTexture(const Path& file) {
    const auto textureOptions = Texture::loadOptions(file);

    const auto dst = replaceExtension(file, ".ktx2");
    if (!Fs::exists(dst) || Fs::last_write_time(file) > Fs::last_write_time(dst)) {
        ktxCompressFile(file, dst, textureOptions.compress);
    } else {
        logger.info("Skipping texture: {}, not modified", file);
    }
}

static void compressShader(const Path& file) {
    const auto dst = replaceExtension(file, ".spirv");
    if (!Fs::exists(dst) || Fs::last_write_time(file) > Fs::last_write_time(dst)) {
        compileGLSLFile(file, dst);
    } else {
        logger.info("Skipping shader: {}, not modified", file);
    }
}

void AssetsManager::compressAssets(const Config& config) {
    try {
        const std::vector<Path> paths = {config.assetsPath / "base"};

        for (const auto& path : paths) {
            if (!Fs::is_directory(path)) {
                continue;
            }

            iterateDir(path / "textures", {".png"}, [&](const Path& file) { compressTexture(file); });
            iterateDir(path / "models", {".png"}, [&](const Path& file) { compressTexture(file); });
            iterateDir(path / "images", {".png"}, [&](const Path& file) { compressTexture(file); });
            iterateDir(path / "shaders", {".glsl"}, [&](const Path& file) { compressShader(file); });
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to compress assets");
    }
}

AssetsManager::AssetsManager(const Config& config) : config{config} {
    instance = this;
    findAssets();
}

void AssetsManager::init(VulkanRenderer& vulkan) {
    try {
        if (!atlas) {
            atlas = std::make_unique<ImageAtlas>(config, vulkan);
        }

        if (!defaultTextures) {
            defaultTextures = std::make_unique<DefaultTextures>();
            defaultTextures->baseColor =
                createTextureOfColor(vulkan, Color4{1.0f, 0.0f, 1.0f, 1.0f}, "default_baseColor");
            defaultTextures->mask = createTextureOfColor(vulkan, Color4{1.0f, 0.0f, 0.0f, 1.0f}, "default_mask");
            defaultTextures->ambient = createTextureOfColor(vulkan, Color4{1.0f, 0.0f, 0.0f, 1.0f}, "default_ambient");
            defaultTextures->emissive =
                createTextureOfColor(vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}, "default_emissive");
            defaultTextures->metallicRoughness =
                createTextureOfColor(vulkan, Color4{0.0f, 0.5f, 0.5f, 1.0f}, "default_metallicRoughness");
            defaultTextures->normal = createTextureOfColor(vulkan, Color4{0.5f, 0.5f, 1.0f, 1.0f}, "default_normal");
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to init assetsManager");
    }
}

void AssetsManager::finalize() {
    atlas->finalize();
}

void AssetsManager::findAssets() {
    try {
        const std::vector<Path> paths = {config.assetsPath / "base"};

        for (const auto& path : paths) {
            addManifest(path);
        }

        for (const auto& path : paths) {
            init(shaders, path / "shaders", {".spirv"});
        }

        for (const auto& path : paths) {
            init(images, path / "images", {".ktx2"});
        }

        for (const auto& path : paths) {
            init(textures, path / "textures", {".ktx2"});
        }

        for (const auto& path : paths) {
            init(models, path / "models", {".gltf"});
        }

        for (const auto& path : paths) {
            init(particlesTypes, path / "particles", {".xml"});
        }

        for (const auto& path : paths) {
            init(blocks, path / "blocks", {".xml"});
        }

        for (const auto& path : paths) {
            init(planetTypes, path / "planets", {".xml"});
        }

        for (const auto& path : paths) {
            init(sounds, path / "sounds", {".ogg"});
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to initialize assetsManager");
    }
}

void AssetsManager::addManifest(const Path& path) {
    try {
        ModManifest manifest{};
        Xml::fromFile(path / "manifest.xml", manifest);
        manifests.push_back(manifest);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load manifest for mod package: \'{}\'", path);
    }
}

AssetsManager& AssetsManager::getInstance() {
    if (!instance) {
        EXCEPTION("No instance initialized of assetsManager");
    }
    return *instance;
}

TexturePtr AssetsManager::createTextureOfColor(VulkanRenderer& vulkan, const Color4& color, const std::string& name) {
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
    vulkan.transitionImageLayout(
        texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return asset;
}

template <typename T>
void AssetsManager::init(Category<T>& assets, const Path& path, const std::set<std::string>& ext) {
    if (!Fs::exists(path)) {
        return;
    }

    if (!Fs::is_directory(path)) {
        EXCEPTION("Assets path: \'{}\' is not a directory", path.string());
    }

    std::set<std::string> processed;

    iterateDir(path, ext, [&](const Path& file) { addAsset(assets, file); });
}

template <typename T> std::shared_ptr<T> AssetsManager::addAsset(Category<T>& assets, const Path& path) {
    // logger.info("Adding asset: '{}'", path);
    auto asset = std::make_shared<T>(path.stem().string(), path);
    assets.insert(asset->getName(), asset);
    loadQueue.emplace_back([=](VulkanRenderer* vulkan, AudioContext* audio) {
        logger.info("Loading asset: '{}'", path);
        asset->load(*this, vulkan, audio);
    });
    return asset;
}

TexturePtr AssetsManager::addTexture(const Path& path) {
    return addAsset(textures, path);
}

ImagePtr AssetsManager::addImage(const std::string& name, const ImageAtlas::Allocation& allocation) {
    auto asset = std::make_shared<Image>(name, allocation);
    images.insert(name, asset);
    return asset;
}

void AssetsManager::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<AssetsManager>("AssetsManager");
    cls["find_planet_type"] = [](AssetsManager& self, const std::string& name) {
        return self.getPlanetTypes().find(name);
    };
    cls["find_image"] = [](AssetsManager& self, const std::string& name) { return self.getImages().find(name); };
    cls["find_texture"] = [](AssetsManager& self, const std::string& name) { return self.getTextures().find(name); };
    cls["find_block"] = [](AssetsManager& self, const std::string& name) { return self.getBlocks().find(name); };
    cls["find_model"] = [](AssetsManager& self, const std::string& name) { return self.getModels().find(name); };
    cls["find_sound"] = [](AssetsManager& self, const std::string& name) { return self.getSounds().find(name); };

    cls["find_all_planet_types"] = [](AssetsManager& self) { return sol::as_table(self.getPlanetTypes().findAll()); };
    cls["find_all_images"] = [](AssetsManager& self) { return sol::as_table(self.getImages().findAll()); };
    cls["find_all_textures"] = [](AssetsManager& self) { return sol::as_table(self.getTextures().findAll()); };
    cls["find_all_blocks"] = [](AssetsManager& self) { return sol::as_table(self.getBlocks().findAll()); };
    cls["find_all_models"] = [](AssetsManager& self) { return sol::as_table(self.getModels().findAll()); };
    cls["find_all_sounds"] = [](AssetsManager& self) { return sol::as_table(self.getSounds().findAll()); };
}
