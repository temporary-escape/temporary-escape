#pragma once

#include "../config.hpp"
#include "block.hpp"
#include "image.hpp"
#include "image_atlas.hpp"
#include "mod_manifest.hpp"
#include "model.hpp"
#include "texture.hpp"
#include "voxel_shape_cache.hpp"
#include <mutex>

namespace Engine {
class Registry {
public:
    struct DefaultTextures {
        TexturePtr baseColor;
        TexturePtr normal;
        TexturePtr emissive;
        TexturePtr ambient;
        TexturePtr metallicRoughness;
    };

    template <typename T> class Category {
    public:
        void insert(const std::string& name, std::shared_ptr<T> asset) {
            assets.insert(std::make_pair(name, asset));
        }

        std::shared_ptr<T> findOrNull(const std::string& name) const {
            const auto it = assets.find(name);
            if (it == assets.end()) {
                return nullptr;
            }

            return it->second;
        }

        std::shared_ptr<T> find(const std::string& name) const {
            const auto ptr = findOrNull(name);
            if (!ptr) {
                EXCEPTION("Asset named: '{}' of type: {} does not exist", name, typeid(T).name());
            }

            return ptr;
        }

        std::vector<std::shared_ptr<T>> findAll() const {
            std::vector<std::shared_ptr<T>> res;
            for (auto& pair : assets) {
                if (pair.second) {
                    res.push_back(pair.second);
                }
            }

            return res;
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<T>> assets;
    };

    static Registry* instance;
    static Registry& getInstance();

    explicit Registry(const Config& config);

    const Category<Texture>& getTextures() const {
        return textures;
    }

    const Category<Block>& getBlocks() const {
        return blocks;
    }

    const Category<Image>& getImages() const {
        return images;
    }

    const Category<Model>& getModels() const {
        return models;
    }

    const std::vector<ModManifest>& getManifests() const {
        return manifests;
    }

    ImageAtlas& getImageAtlas() {
        if (!atlas) {
            EXCEPTION("Image atlas was not initialized");
        }
        return *atlas;
    }

    TexturePtr addTexture(const Path& path) {
        return addAsset(textures, path);
    }

    void init();
    void load(VulkanDevice& vulkan);
    bool isReady();

    const VoxelShapeCache& getVoxelShapeCache() const {
        if (!voxelShapeCache) {
            EXCEPTION("Voxel shape cache was not initialized");
        }
        return *voxelShapeCache;
    }

    const DefaultTextures& getDefaultTextures() const {
        if (!voxelShapeCache) {
            EXCEPTION("Default textures were not initialized");
        }
        return *defaultTextures;
    }

private:
    void addManifest(const Path& path);
    TexturePtr createTextureOfColor(VulkanDevice& vulkan, const Color4& color, const std::string& name);

    template <typename T> void init(Category<T>& assets, const Path& path, const std::set<std::string>& ext) {
        if (!Fs::exists(path)) {
            return;
        }

        if (!Fs::is_directory(path)) {
            EXCEPTION("Assets path: \'{}\' is not a directory", path.string());
        }

        iterateDir(path, ext, [&](const Path& file) { addAsset(assets, file); });
    }

    template <typename T> std::shared_ptr<T> addAsset(Category<T>& assets, const Path& path) {
        Log::i("Registry", "Adding asset: '{}'", path);
        auto asset = std::make_shared<T>(path.stem().string(), path);
        assets.insert(asset->getName(), asset);
        std::lock_guard<std::mutex> lock{loadMutex};
        loadQueue.emplace_back([=](VulkanDevice& vulkan) {
            Log::i("Registry", "Loading asset: '{}'", path);
            asset->load(*this, vulkan);
        });
        return asset;
    }

    const Config& config;
    std::unique_ptr<VoxelShapeCache> voxelShapeCache;
    std::unique_ptr<ImageAtlas> atlas;
    std::unique_ptr<DefaultTextures> defaultTextures;
    Category<Texture> textures;
    Category<Block> blocks;
    Category<Image> images;
    Category<Model> models;
    std::vector<ModManifest> manifests;
    std::mutex loadMutex;
    std::list<std::function<void(VulkanDevice&)>> loadQueue;
};
} // namespace Engine