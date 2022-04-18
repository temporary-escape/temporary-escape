#include "AssetManager.hpp"

#include "../Utils/Exceptions.hpp"
#include "../Utils/StringUtils.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <wrenbind17/wrenbind17.hpp>

#define CMP "AssetManager"

using namespace Engine;

AssetManager* AssetManager::instance = nullptr;

AssetManager::AssetManager(const Config& config, Canvas2D& canvas, TextureCompressor& textureCompressor)
    : config(config), canvas(canvas), textureCompressor(textureCompressor), textureAtlas(config, canvas) {
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

AssetImagePtr AssetManager::addImage(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto image = std::make_shared<AssetImage>(mod, baseName, path);
        add(image);
        return image;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add image: '{}'", path.string());
    }
}

AssetImagePtr AssetManager::addImage(const Manifest& mod, const std::string& name, Canvas2D::Image image) {
    try {
        auto asset = std::make_shared<AssetImage>(mod, name, image);
        add(asset);
        return asset;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add image: '{}'", name);
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

AssetEntityPtr AssetManager::addEntity(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto entity = std::make_shared<AssetEntity>(mod, baseName, path);
        add(entity);
        return entity;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add entity: '{}'", path.string());
    }
}

AssetSectorPtr AssetManager::addSector(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto sector = std::make_shared<AssetSector>(mod, baseName, path);
        add(sector);
        return sector;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add sector: '{}'", path.string());
    }
}

AssetParticlesPtr AssetManager::addParticles(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto particles = std::make_shared<AssetParticles>(mod, baseName, path);
        add(particles);
        return particles;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add particles: '{}'", path.string());
    }
}

AssetTurretPtr AssetManager::addTurret(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto turret = std::make_shared<AssetTurret>(mod, baseName, path);
        add(turret);
        return turret;
    } catch (...) {
        EXCEPTION_NESTED("Failed to add turret: '{}'", path.string());
    }
}

void AssetManager::add(AssetPtr asset) {
    Log::i(CMP, "Adding asset '{}/{}'", asset->getMod().name, asset->getName());
    assets.insert(std::make_pair(asset->getName(), asset));
}

AssetLoadQueue AssetManager::getLoadQueue() {
    AssetLoadQueue loadQueue;

    for (const auto& [key, asset] : assets) {
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

Canvas2D::Image AssetManager::addToAtlas(const Vector2i& size, const void* pixels) {
    auto [pos, handle] = textureAtlas.add(size, pixels);
    Canvas2D::Image image{};
    image.handle = handle;
    image.pos = pos;
    image.atlasSize = Vector2i{config.imageAtlasSize};
    image.size = size;
    return image;
}

Canvas2D::FontHandle AssetManager::createFontHandle(const Path& path) {
    return canvas.loadFont(path);
}

void AssetManager::runAllScripts(wrenbind17::VM& vm) {
    for (const auto& [_, asset] : assets) {
        auto ptr = std::dynamic_pointer_cast<AssetEntity>(asset);
        if (ptr) {
            ptr->run(vm);
        }
    }
}
