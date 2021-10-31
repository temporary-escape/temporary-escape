#pragma once
#include "../Config.hpp"
#include "../Library.hpp"
#include "../Utils/Exceptions.hpp"
#include "Asset.hpp"
#include "AssetFontFamily.hpp"
#include "AssetModel.hpp"
#include "TextureCompressor.hpp"

#include <queue>

namespace Scissio {
using AssetPtrMap = std::unordered_map<std::string, AssetPtr>;
using AssetLoadQueue = std::queue<std::function<void()>>;

class Canvas2D;

class SCISSIO_API AssetManager {
public:
    explicit AssetManager(const Config& config, TextureCompressor& textureCompressor);
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
    AssetLoadQueue getLoadQueue();

    Texture2D compressTexture(Texture2D& source, const Vector2i& targetSize, PixelType target);

    static AssetManager& singleton();

private:
    static AssetManager* instance;

    void add(AssetPtr asset);

    const Config& config;
    TextureCompressor& textureCompressor;
    AssetPtrMap assets;
};
} // namespace Scissio
