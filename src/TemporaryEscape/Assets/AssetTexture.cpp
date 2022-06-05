#include "AssetTexture.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"

#define CMP "AssetTexture"

using namespace Engine;

static inline const std::unordered_map<TextureType, PixelType> textureTypeToPixelType = {
    {TextureType::BaseColor, PixelType::CompressedRGBAS3tcDxt5},
    {TextureType::Emissive, PixelType::CompressedRGBAS3tcDxt1},
    {TextureType::Normals, PixelType::CompressedRGRgtc2},
    {TextureType::MetallicRoughness, PixelType::CompressedRGBAS3tcDxt1},
    {TextureType::AmbientOcclusion, PixelType::CompressedRGBAS3tcDxt1},
};

static bool endsWith(std::string const& value, std::string const& ending) {
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

AssetTexture::AssetTexture(const Manifest& mod, std::string name, const Path& path, const TextureType type)
    : Asset(mod, std::move(name)), path(path), type(type), texture{NO_CREATE} {

    const auto optionsPath = path.parent_path() / (path.stem().string() + std::string(".yml"));
    if (Fs::exists(optionsPath)) {
        try {
            options.fromYaml(optionsPath);
        } catch (...) {
            EXCEPTION_NESTED("Failed to load texture options from: '{}'", optionsPath.string());
        }
    }

    const auto filename = path.stem().string();
    if (endsWith(filename, "_diff")) {
        this->type = TextureType::BaseColor;
    } else if (endsWith(filename, "_norm")) {
        this->type = TextureType::Normals;
    } else if (endsWith(filename, "_meta")) {
        this->type = TextureType::MetallicRoughness;
    } else if (endsWith(filename, "_ao")) {
        this->type = TextureType::AmbientOcclusion;
    } else if (endsWith(filename, "_emis")) {
        this->type = TextureType::Emissive;
    } else {
        this->type = TextureType::Generic;
    }
}

void AssetTexture::load(AssetManager& assetManager, bool noGraphics) {
    if (noGraphics) {
        return;
    }

    try {
        PngImporter image(path);

        auto size = image.getSize();
        auto temp = Texture2D{};
        temp.setStorage(0, size, image.getPixelType());
        temp.setPixels(0, {0, 0}, image.getSize(), image.getPixelType(), image.getData());
        temp.setMipMapLevel(0, 0);

        if (type != TextureType::Generic) {
            texture = assetManager.compressTexture(temp, image.getSize(), textureTypeToPixelType.at(type));
        } else {
            texture = std::move(temp);
            texture.generateMipmaps();
        }

        if (options.filtering.has_value()) {
            texture.setFiltering(options.filtering.value().minification, options.filtering.value().magnification);
        }
        if (options.wrapping.has_value()) {
            texture.setWrapping(options.wrapping.value().horizontal, options.wrapping.value().vertical);
        } else {
            texture.setWrapping(TextureWrapping::Repeat, TextureWrapping::Repeat);
        }

    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

std::shared_ptr<AssetTexture> AssetTexture::from(const std::string& name) {
    return AssetManager::singleton().find<AssetTexture>(name);
}
