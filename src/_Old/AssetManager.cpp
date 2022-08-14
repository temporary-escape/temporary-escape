#include "AssetManager.hpp"

#include "../Utils/Exceptions.hpp"
#include "../Utils/StringUtils.hpp"

#define CMP "AssetManager"

using namespace Engine;

AssetManager* AssetManager::instance = nullptr;

AssetManager::AssetManager(const Config& config) : config(config) {
    instance = this;
}

AssetManager& AssetManager::singleton() {
    if (!instance) {
        EXCEPTION("Asset manager has not been initialized");
    }
    return *instance;
}

AssetModelPtr AssetManager::addModel(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto model = std::make_shared<AssetModel>(mod, baseName, path);
        add(model);
        return model;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add model: '{}'", path.string());
    }
}

AssetShapePtr AssetManager::addShape(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto shape = std::make_shared<AssetShape>(mod, baseName, path);
        add(shape);
        return shape;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add shape: '{}'", path.string());
    }
}

AssetTexturePtr AssetManager::addTexture(const Manifest& mod, const Path& path, const TextureType type) {
    try {
        const auto baseName = path.stem().string();
        auto texture = std::make_shared<AssetTexture>(mod, baseName, path, type);
        add(texture);
        return texture;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add texture: '{}'", path.string());
    }
}

AssetBlockPtr AssetManager::addBlock(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto block = std::make_shared<AssetBlock>(mod, baseName, path);
        add(block);
        return block;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add block: '{}'", path.string());
    }
}

void AssetManager::add(AssetPtr asset) {
    Log::i(CMP, "Adding asset '{}/{}'", asset->getMod().name, asset->getName());
    assets.insert(std::make_pair(asset->getName(), asset));
}

AssetLoadQueue AssetManager::getLoadQueue(bool noGraphics) {
    AssetLoadQueue loadQueue;

    for (const auto& [key, asset] : assets) {
        loadQueue.push([this, noGraphics, asset = asset]() {
            try {
                Log::d(CMP, "Loading asset '{}/{}'", asset->getMod().name, asset->getName());
                asset->load(*this, noGraphics);
            } catch (...) {
                EXCEPTION_NESTED("Failed to load asset: '{}'", asset->getName());
            }
        });
    }

    return loadQueue;
}
