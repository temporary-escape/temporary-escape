#include "BasicTexture.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

BasicTexture::BasicTexture(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path), texture{NO_CREATE} {
}

void BasicTexture::load(AssetManager& assetManager) {
    Log::d("Loading basic texture: '{}'", getName());

    try {
        PngImporter image(path);

        auto size = image.getSize();

        texture = Texture2D{};
        texture.setStorage(0, size, image.getPixelType());
        texture.setPixels(0, {0, 0}, image.getSize(), image.getPixelType(), image.getData());

    } catch (...) {
        EXCEPTION_NESTED("Failed to load pbr texture: '{}'", getName());
    }
}
