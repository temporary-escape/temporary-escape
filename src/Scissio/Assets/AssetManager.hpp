#pragma once
#include "../Config.hpp"
#include "../Library.hpp"
#include "../Utils/Exceptions.hpp"
#include "Asset.hpp"
#include "AssetAsteroid.hpp"
#include "AssetFontFamily.hpp"
#include "AssetImage.hpp"
#include "AssetModel.hpp"
#include "AssetPlanet.hpp"
#include "TextureAtlas.hpp"
#include "TextureCompressor.hpp"
#include <functional>
#include <queue>

namespace Scissio {
using AssetPtrMap = std::unordered_map<std::string, AssetPtr>;
using AssetLoadQueue = std::queue<std::function<void()>>;

class Canvas2D;

class SCISSIO_API AssetManager {
public:
    explicit AssetManager(const Config& config, Canvas2D& canvas, TextureCompressor& textureCompressor);
    virtual ~AssetManager() = default;

    template <typename T> std::shared_ptr<T> findOrNull(const std::string& name) {
        const auto it = assets.find(name);
        if (it == assets.end()) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<T>(it->second);
    }

    template <typename T> std::shared_ptr<T> find(const std::string& name) {
        const auto ptr = findOrNull<T>(name);
        if (!ptr) {
            EXCEPTION("Asset '{}' does not exist", name);
        }

        return ptr;
    }

    template <typename T> std::vector<std::shared_ptr<T>> findAll() {
        std::vector<std::shared_ptr<T>> res;
        for (auto& pair : assets) {
            auto ptr = std::dynamic_pointer_cast<T>(pair.second);
            if (ptr) {
                res.push_back(ptr);
            }
        }

        return res;
    }

    AssetFontFacePtr addFontFace(const Manifest& mod, const Path& path);
    AssetModelPtr addModel(const Manifest& mod, const Path& path);
    AssetTexturePtr addTexture(const Manifest& mod, const Path& path, TextureType type);
    AssetImagePtr addImage(const Manifest& mod, const Path& path);
    AssetPlanetPtr addPlanet(const Manifest& mod, const Path& path);
    AssetAsteroidPtr addAsteroid(const Manifest& mod, const Path& path);
    AssetLoadQueue getLoadQueue();

    Texture2D compressTexture(Texture2D& source, const Vector2i& targetSize, PixelType target);
    Canvas2D::Image addToAtlas(const Vector2i& size, const void* pixels);
    Canvas2D::FontHandle createFontHandle(const Path& path);

    static AssetManager& singleton();

private:
    static AssetManager* instance;

    void add(AssetPtr asset);

    const Config& config;
    Canvas2D& canvas;
    TextureCompressor& textureCompressor;
    TextureAtlas textureAtlas;
    AssetPtrMap assets;
};
} // namespace Scissio
