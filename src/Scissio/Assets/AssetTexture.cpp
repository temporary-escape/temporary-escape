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

    } catch (...) {
        EXCEPTION_NESTED("Failed to load texture: '{}'", getName());
    }
}

template <> void Xml::Node::convert<AssetTexturePtr>(AssetTexturePtr& value) const {
    value = AssetManager::singleton().find<AssetTexture>(this->asString());
}

MSGPACK_UNPACK_FUNC(AssetTexturePtr) {
    if (o.type == msgpack::type::STR) {
        const auto name = o.as<std::string>();
        v = AssetManager::singleton().find<AssetTexture>(name);
    } else {
        throw msgpack::type_error();
    }

    return o;
}

MSGPACK_PACK_FUNC(AssetTexturePtr) {
    if (v) {
        const auto& name = v->getName();
        o.pack_str(static_cast<uint32_t>(name.size()));
        o.pack_str_body(name.c_str(), static_cast<uint32_t>(name.size()));
    } else {
        o.pack_nil();
    }

    return o;
}
