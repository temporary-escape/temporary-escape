#include "IconAtlas.hpp"

#include "../Graphics/PixelType.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/PngImporter.hpp"

using namespace Scissio;

IconAtlas::IconAtlas(const Manifest& mod, std::string name, Path path)
    : Asset(mod, std::move(name)), path(std::move(path)) {
}

void IconAtlas::load(AssetManager& assetManager) {
    (void)assetManager;

    try {
        Log::d("Loading icon atlas: '{}'", getName());

        PngImporter image(path);
        if (image.getPixelType() != PixelType::Rgba8u) {
            EXCEPTION("Icon atlas '{}' file: '{}' must have RGBA8 format", getName(), path.string());
        }

        size = image.getSize();
        texture = Texture2D{};
        texture.setStorage(0, size, image.getPixelType());
        texture.setPixels(0, {0, 0}, image.getSize(), image.getPixelType(), image.getData());
        texture.generateMipmaps();
    } catch (...) {
        EXCEPTION_NESTED("Failed to load icon atlas: '{}'", getName());
    }
}

void IconAtlas::add(std::string name, IconPtr icon) {
    icons.insert(std::make_pair(std::move(name), std::move(icon)));
}

const IconPtr& IconAtlas::get(const std::string& name) const {
    try {
        return icons.at(name);
    } catch (const std::out_of_range& e) {
        (void)e;
        EXCEPTION("Icon '{}' of icon atlas '{}' not found", name, getName());
    }
}
