#include "AssetManager.hpp"

#include "../Utils/Exceptions.hpp"
#include "../Utils/StringUtils.hpp"

#include <fstream>
#include <iostream>
#include <set>

#define CMP "AssetManager"

using namespace Scissio;

AssetManager* AssetManager::instance = nullptr;

AssetManager::AssetManager(const Config& config, Canvas2D& canvas, TextureCompressor& textureCompressor)
    : config(config), canvas(canvas), textureCompressor(textureCompressor) {
    instance = this;
}

AssetManager& AssetManager::singleton() {
    if (!instance) {
        EXCEPTION("Asset manager has not been initialized");
    }
    return *instance;
}

AssetFontFacePtr AssetManager::addFontFace(const Manifest& mod, const Path& path) {
    static const std::set<std::string> SUFFIXES = {
        "bold",          "bolditalic", "light",          "lightitalic", "regular",
        "regularitalic", "semibold",   "semibolditalic", "thin",        "thinitalic",
    };

    try {
        const auto baseName = path.stem().string();
        const auto tokens = splitLast(baseName, "-");
        if (tokens.size() != 2) {
            EXCEPTION("Font '{}' has incorrect name, must be '<font family>-<font face>'", path.string());
        }

        const auto& fontFamilyName = tokens.front();
        const auto& faceName = tokens.back();

        if (SUFFIXES.find(faceName) == SUFFIXES.end()) {
            EXCEPTION("Font '{}' has unknown face name '{}'", path.string(), faceName);
        }

        const auto it = assets.find(fontFamilyName);
        AssetFontFamilyPtr fontFamily;
        if (it != assets.end()) {
            fontFamily = std::dynamic_pointer_cast<AssetFontFamily>(it->second);
        } else {
            fontFamily = std::make_shared<AssetFontFamily>(mod, fontFamilyName);
            add(fontFamily);
        }

        auto fontFace = std::make_shared<AssetFontFace>(mod, config, path);
        fontFamily->add(faceName, fontFace);

        add(fontFace);

        return fontFace;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add font face: '{}'", path.string());
    }
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

AssetPlanetPtr AssetManager::addPlanet(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto planet = std::make_shared<AssetPlanet>(mod, baseName, path);
        add(planet);
        return planet;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add planet: '{}'", path.string());
    }
}

void AssetManager::add(AssetPtr asset) {
    Log::i(CMP, "Adding asset '{}/{}'", asset->getMod().name, asset->getName());
    assets.insert(std::make_pair(asset->getName(), asset));
}

AssetLoadQueue AssetManager::getLoadQueue() {
    AssetLoadQueue loadQueue;

    for (const auto [key, asset] : assets) {
        loadQueue.push([this, asset = asset]() {
            try {
                Log::d(CMP, "Loading asset '{}/{}'", asset->getMod().name, asset->getName());
                asset->load(*this);
            } catch (...) {
                EXCEPTION_NESTED("Failed to load asset: '{}'", asset->getName());
            }
        });
    }

    return loadQueue;
}

Texture2D AssetManager::compressTexture(Texture2D& source, const Vector2i& targetSize, PixelType target) {
    return textureCompressor.convert(source, targetSize, target);
}

Canvas2D::FontHandle AssetManager::createFontHandle(const Path& path) {
    return canvas.loadFont(path);
}
