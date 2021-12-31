#include "AssetTexture.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

static inline const std::unordered_map<TextureType, PixelType> textureTypeToPixelType = {
    {TextureType::BaseColor, PixelType::CompressedRGBAS3tcDxt5},
    {TextureType::Emissive, PixelType::CompressedRGBAS3tcDxt1},
    {TextureType::Normals, PixelType::CompressedRGRgtc2},
    {TextureType::MetallicRoughness, PixelType::CompressedRGBAS3tcDxt1},
    {TextureType::AmbientOcclusion, PixelType::CompressedRGBAS3tcDxt1},
};

AssetTexture::AssetTexture(const Manifest& mod, std::string name, const Path& path, const TextureType type)
    : Asset(mod, std::move(name)), path(path), type(type), texture{NO_CREATE} {
}

void AssetTexture::load(AssetManager& assetManager) {
    try {
        PngImporter image(path);

        auto size = image.getSize();
        auto temp = Texture2D{};
        temp.setStorage(0, size, image.getPixelType());
        temp.setPixels(0, {0, 0}, image.getSize(), image.getPixelType(), image.getData());

        if (type != TextureType::Generic) {
            texture = assetManager.compressTexture(temp, image.getSize(), textureTypeToPixelType.at(type));
        } else {
            texture = std::move(temp);
            texture.generateMipmaps();
        }

        Options options{};
        const auto optionsPath = path.parent_path() / (path.stem().string() + std::string(".xml"));
        if (Fs::exists(optionsPath)) {
            try {
                Xml::Document(optionsPath).getRoot().convert(options);
            } catch (...) {
                EXCEPTION_NESTED("Failed to load texture options from: '{}'", optionsPath.string());
            }
        }

        texture.setFiltering(options.filtering.minification, options.filtering.magnification);
        texture.setWrapping(options.wrapping.horizontal, options.wrapping.vertical);

    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

std::shared_ptr<AssetTexture> AssetTexture::from(const std::string& name) {
    return AssetManager::singleton().find<AssetTexture>(name);
}

void Xml::Adaptor<AssetTexture::Options::Wrapping>::convert(const Xml::Node& n, AssetTexture::Options::Wrapping& v) {
    n.child("horizontal").convert(v.horizontal);
    n.child("vertical").convert(v.vertical);
}

void Xml::Adaptor<AssetTexture::Options::Filtering>::convert(const Xml::Node& n, AssetTexture::Options::Filtering& v) {
    n.child("magnification").convert(v.magnification);
    n.child("minification").convert(v.minification);
}

void Xml::Adaptor<AssetTexture::Options>::convert(const Xml::Node& n, AssetTexture::Options& v) {
    n.child("filtering").convert(v.filtering);
    n.child("wrapping").convert(v.wrapping);
}

void Xml::Adaptor<AssetTexturePtr>::convert(const Xml::Node& n, AssetTexturePtr& v) {
    v = AssetTexture::from(n.asString());
}
