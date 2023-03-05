#pragma once

#include "../config.hpp"
#include "block.hpp"
#include "image.hpp"
#include "image_atlas.hpp"
#include "mod_manifest.hpp"
#include "model.hpp"
#include "planet_type.hpp"
#include "texture.hpp"
#include "voxel_shape_cache.hpp"
#include <mutex>

namespace Engine {
class Registry {
public:
    using LoadQueue = std::list<std::function<void(VulkanRenderer&)>>;

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

    const Category<PlanetType>& getPlanetTypes() const {
        return planetTypes;
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

    TexturePtr addTexture(const Path& path);
    ImagePtr addImage(const std::string& name, const ImageAtlas::Allocation& allocation);

    void findAssets();
    void init(VulkanRenderer& vulkan);
    void finalize();

    const DefaultTextures& getDefaultTextures() const {
        if (!defaultTextures) {
            EXCEPTION("Default textures were not initialized");
        }
        return *defaultTextures;
    }

    const LoadQueue& getLoadQueue() const {
        return loadQueue;
    }

private:
    void addManifest(const Path& path);
    TexturePtr createTextureOfColor(VulkanRenderer& vulkan, const Color4& color, const std::string& name);

    template <typename T> void init(Category<T>& assets, const Path& path, const std::set<std::string>& ext);

    template <typename T> std::shared_ptr<T> addAsset(Category<T>& assets, const Path& path);

    const Config& config;
    std::unique_ptr<ImageAtlas> atlas;
    std::unique_ptr<DefaultTextures> defaultTextures;
    Category<Texture> textures;
    Category<Block> blocks;
    Category<Image> images;
    Category<Model> models;
    Category<PlanetType> planetTypes;
    std::vector<ModManifest> manifests;
    LoadQueue loadQueue;
};
} // namespace Engine
