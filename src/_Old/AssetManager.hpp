#pragma once
#include "../Config.hpp"
#include "../Library.hpp"
#include "../Utils/Exceptions.hpp"
#include "Asset.hpp"
#include "AssetBlock.hpp"
#include "AssetImage.hpp"
#include "AssetModel.hpp"
#include "AssetShape.hpp"
#include <functional>
#include <queue>

namespace Engine {
using AssetPtrMap = std::unordered_map<std::string, AssetPtr>;
using AssetLoadQueue = std::queue<std::function<void()>>;

class ENGINE_API AssetManager {
public:
    explicit AssetManager(const Config& config);
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

    AssetModelPtr addModel(const Manifest& mod, const Path& path);
    AssetShapePtr addShape(const Manifest& mod, const Path& path);
    AssetTexturePtr addTexture(const Manifest& mod, const Path& path, TextureType type);
    AssetBlockPtr addBlock(const Manifest& mod, const Path& path);
    AssetLoadQueue getLoadQueue(bool noGraphics);

    static AssetManager& singleton();

private:
    static AssetManager* instance;

    void add(AssetPtr asset);

    const Config& config;
    AssetPtrMap assets;
};
} // namespace Engine
