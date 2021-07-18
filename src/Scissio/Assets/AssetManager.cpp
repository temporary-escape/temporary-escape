#include "AssetManager.hpp"

#include "../Utils/Exceptions.hpp"
#include "../Utils/GltfImporter.hpp"
#include "../Utils/StringUtils.hpp"
#include "BasicTexture.hpp"
#include "FontFamily.hpp"
#include "IconAtlas.hpp"
#include "Model.hpp"
#include "PbrTexture.hpp"

#include <fstream>
#include <iostream>
#include <set>

using namespace Scissio;

AssetManager* AssetManager::instance = nullptr;

AssetManager::AssetManager(const Config& config, Canvas2D& canvas, TextureCompressor& textureCompressor,
                           Renderer& renderer)
    : canvas(canvas), imageAtlas(config), textureCompressor(textureCompressor), renderer(renderer) {

    instance = this;
}

AssetManager& AssetManager::singleton() {
    if (!instance) {
        EXCEPTION("Asset manager has not been initialized");
    }
    return *instance;
}

template <> FontFacePtr AssetManager::load<FontFace>(const Manifest& mod, const Path& path) {
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

        const auto fontFamilyName = tokens.front();
        const auto faceName = tokens.back();

        if (SUFFIXES.find(faceName) == SUFFIXES.end()) {
            EXCEPTION("Font '{}' has unknown face name '{}'", path.string(), faceName);
        }

        const auto it = assets.find(fontFamilyName);
        FontFamilyPtr fontFamily;
        if (it != assets.end()) {
            fontFamily = std::dynamic_pointer_cast<FontFamily>(it->second);
        } else {
            fontFamily = std::make_shared<FontFamily>(mod, fontFamilyName);
            add(fontFamily);
        }

        auto fontFace = std::make_shared<FontFace>(mod, path);
        fontFamily->add(faceName, fontFace);

        add(fontFace);

        return fontFace;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load font face: '{}'", path.string());
    }
}

template <> IconAtlasPtr AssetManager::load<IconAtlas>(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        const auto dir = path.parent_path();

        const auto xmlPath = dir / Path(baseName + ".xml");
        const auto pngPath = dir / Path(baseName + ".png");

        if (!Fs::exists(xmlPath)) {
            EXCEPTION("Icon atlas '{}' does not have an xml file", path.string());
        }
        if (!Fs::exists(pngPath)) {
            EXCEPTION("Icon atlas '{}' does not have a png file", path.string());
        }

        auto iconAtlas = std::make_shared<IconAtlas>(mod, baseName, pngPath);
        add(iconAtlas);

        const auto atlasSize = iconAtlas->getSize();

        Xml::Document xml(xmlPath);

        for (const auto& node : xml.getRoot()) {
            const Vector2i size = {
                node.child("width").asLong(),
                node.child("height").asLong(),
            };
            const Vector2i pos = {
                node.child("x").asLong(),
                node.child("y").asLong(),
            };
            const auto name = node.child("name").asString();

            // const auto p = Vector2{pos.x, 4096 - pos.y - size.y};

            auto icon = std::make_shared<Icon>(mod, baseName + "-" + name, *iconAtlas, pos, size);
            iconAtlas->add(name, icon);
            add(icon);
        }

        return iconAtlas;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load icon atlas: '{}'", path.string());
    }
}

template <> ModelPtr AssetManager::load<Model>(const Manifest& mod, const Path& path) {
    try {
        GltfImporter importer(path);
        const auto dir = path.parent_path();

        auto resolveTexture = [&mod, dir, this](const std::string& filename, const PbrTextureType type) {
            const auto path = dir / Path(filename);
            const auto baseName = path.stem().string();
            if (assets.find(baseName) == assets.end()) {
                try {
                    add(std::make_shared<PbrTexture>(mod, baseName, path, type));
                } catch (...) {
                    EXCEPTION_NESTED("Failed to load texture: '{}'", path.string());
                }
            }
        };

        for (const auto& material : importer.getMaterials()) {
            if (material.ambientOcclusionTexture.has_value()) {
                resolveTexture(material.ambientOcclusionTexture.value().getUri(), PbrTextureType::AmbientOcclusion);
            }
            if (material.metallicRoughnessTexture.has_value()) {
                resolveTexture(material.metallicRoughnessTexture.value().getUri(), PbrTextureType::MetallicRoughness);
            }
            if (material.emissiveTexture.has_value()) {
                resolveTexture(material.emissiveTexture.value().getUri(), PbrTextureType::Emissive);
            }
            if (material.baseColorTexture.has_value()) {
                resolveTexture(material.baseColorTexture.value().getUri(), PbrTextureType::BaseColor);
            }
            if (material.normalTexture.has_value()) {
                resolveTexture(material.normalTexture.value().getUri(), PbrTextureType::Normals);
            }
        }

        const auto baseName = path.stem().string();
        auto model = std::make_shared<Model>(mod, baseName, path);
        add(model);
        return model;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load model: '{}'", path.string());
    }
}

template <> BasicTexturePtr AssetManager::load<BasicTexture>(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto texture = std::make_shared<BasicTexture>(mod, baseName, path);
        add(texture);
        return texture;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", path.string());
    }
}

template <> ImagePtr AssetManager::load<Image>(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        auto image = std::make_shared<Image>(mod, baseName, imageAtlas, path);
        add(image);
        return image;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", path.string());
    }
}
void AssetManager::add(AssetPtr asset) {
    assets.insert(std::make_pair(asset->getName(), asset));
    loadQueue.push([this, asset]() {
        try {
            asset->load(*this);
        } catch (...) {
            EXCEPTION_NESTED("Failed to load asset: '{}'", asset->getName());
        }
    });
}

ImagePtr AssetManager::generateImage(const Manifest& mod, const std::string& name, const Vector2i& size,
                                     std::unique_ptr<char[]> pixels) {
    const auto node = imageAtlas.reserve(size);
    node->texture->setPixels(0, node->pos, node->size, PixelType::Rgba8u, pixels.get());
    node->texture->unbind();

    auto image = std::make_shared<Image>(mod, name, imageAtlas, *node->texture, node->pos, node->size);

    add(image);
    image->load(*this);
    return image;
}
