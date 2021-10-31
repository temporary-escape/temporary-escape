#include "PbrTexture.hpp"
#include "../Graphics/TextureCompressor.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

static inline const std::unordered_map<PbrTextureType, PixelType> TEXTURE_TYPE_TO_PIXEL_TYPE = {
    {PbrTextureType::BaseColor, PixelType::CompressedRGBAS3tcDxt5},
    {PbrTextureType::Emissive, PixelType::CompressedRGBAS3tcDxt1},
    {PbrTextureType::Normals, PixelType::CompressedRGRgtc2},
    {PbrTextureType::MetallicRoughness, PixelType::CompressedRGBAS3tcDxt1},
    {PbrTextureType::AmbientOcclusion, PixelType::CompressedRGBAS3tcDxt1},
};

PbrTexture::PbrTexture(const Manifest& mod, std::string name, const Path& path, const PbrTextureType type)
    : Asset(mod, std::move(name)), path(path), type(type), texture{NO_CREATE} {
}

void PbrTexture::load(AssetManager& assetManager) {
    Log::d("Loading pbr texture: '{}'", getName());

    try {
        PngImporter image(path);
        /*if (image.getPixelType() != PixelType::Rgba8u) {
            EXCEPTION("Icon atlas '{}' file: '{}' must have RGBA8 format", getName(), path.string());
        }*/

        auto size = image.getSize();
        auto temp = Texture2D{};
        temp.setStorage(0, size, image.getPixelType());
        temp.setPixels(0, {0, 0}, image.getSize(), image.getPixelType(), image.getData());
        // temp.generateMipmaps();

        texture =
            assetManager.getTextureCompressor().convert(temp, image.getSize(), TEXTURE_TYPE_TO_PIXEL_TYPE.at(type));

    } catch (...) {
        EXCEPTION_NESTED("Failed to load pbr texture: '{}'", getName());
    }
}
