#pragma once

#include "../Config.hpp"
#include "../Graphics/MaterialTextures.hpp"
#include "Block.hpp"
#include "Image.hpp"
#include "ImageAtlas.hpp"
#include "ModManifest.hpp"
#include "Model.hpp"
#include "ParticlesType.hpp"
#include "PlanetType.hpp"
#include "ShipTemplate.hpp"
#include "Sound.hpp"
#include "Texture.hpp"
#include "Turret.hpp"
#include "VoxelShapeCache.hpp"
#include <mutex>

namespace Engine {
class ENGINE_API AssetsManager {
public:
    static constexpr auto defaultTextureDiffName = "texture_default_diff";
    static constexpr auto defaultTextureEmisName = "texture_default_emis";
    static constexpr auto defaultTextureNormName = "texture_default_norm";
    static constexpr auto defaultTextureMetaName = "texture_default_meta";
    static constexpr auto defaultTextureAoName = "texture_default_ao";
    static constexpr auto defaultTextureMaskName = "texture_default_mask";

    using LoadQueue = std::list<std::function<void(VulkanRenderer*, AudioContext*)>>;

    struct DefaultTextures {
        TexturePtr baseColor;
        TexturePtr normal;
        TexturePtr emissive;
        TexturePtr ambient;
        TexturePtr metallicRoughness;
        TexturePtr mask;
    };

    template <typename T> class Category {
    public:
        using Map = std::unordered_map<std::string, std::shared_ptr<T>>;

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

        typename Map::iterator begin() {
            return assets.begin();
        }

        typename Map::iterator end() {
            return assets.end();
        }

        typename Map::const_iterator begin() const {
            return assets.begin();
        }

        typename Map::const_iterator end() const {
            return assets.end();
        }

    private:
        Map assets;
    };

    static void compressAssets(const Config& config);

    static AssetsManager* instance;
    static AssetsManager& getInstance();

    explicit AssetsManager(const Config& config);

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

    const Category<Sound>& getSounds() const {
        return sounds;
    }

    const Category<ParticlesType>& getParticlesTypes() const {
        return particlesTypes;
    }

    const Category<ShipTemplate>& getShipTemplates() const {
        return shipTemplates;
    }

    const Category<Turret>& getTurrets() const {
        return turrets;
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

    MaterialTextures& getMaterialTextures() {
        return *materialTextures;
    }
    const MaterialTextures& getMaterialTextures() const {
        return *materialTextures;
    }
    const VulkanBuffer& getBlockMaterialsUbo() const {
        return blockMaterialsUbo;
    }
    const VulkanBuffer& getParticlesTypesUbo() const {
        return particlesTypesUbo;
    }

    std::tuple<size_t, Block::MaterialUniform*> addBlockMaterial();
    std::tuple<size_t, ParticlesType::Uniform*> addParticleType();

private:
    void addManifest(const Path& path);
    TexturePtr createTextureOfColor(VulkanRenderer& vulkan, const Color4& color, const std::string& name);
    template <typename T> void init(Category<T>& assets, const Path& path, const std::set<std::string>& ext);
    template <typename T> std::shared_ptr<T> addAsset(Category<T>& assets, const Path& path);
    template <typename T> void addToLoadQueue(Category<T>& assets);

    const Config& config;
    std::unique_ptr<ImageAtlas> atlas;
    std::unique_ptr<DefaultTextures> defaultTextures;
    std::unique_ptr<MaterialTextures> materialTextures;
    Category<Texture> textures;
    Category<Block> blocks;
    Category<Image> images;
    Category<Model> models;
    Category<PlanetType> planetTypes;
    Category<Sound> sounds;
    Category<ParticlesType> particlesTypes;
    Category<ShipTemplate> shipTemplates;
    Category<Turret> turrets;
    std::vector<ModManifest> manifests;
    std::vector<Block::MaterialUniform> blockMaterials;
    std::vector<ParticlesType::Uniform> particlesTypeUniforms;
    VulkanBuffer blockMaterialsUbo;
    VulkanBuffer particlesTypesUbo;
    LoadQueue loadQueue;
};
} // namespace Engine
