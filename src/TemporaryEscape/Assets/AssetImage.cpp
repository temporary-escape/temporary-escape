#include "AssetImage.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetImage::AssetImage(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

AssetImage::AssetImage(const Manifest& mod, std::string name, Canvas2D::Image image)
    : Asset(mod, std::move(name)), image(image) {
}

void AssetImage::load(AssetManager& assetManager, bool noGraphics) {
    if (path.empty() || noGraphics) {
        return;
    }

    try {
        PngImporter png(path);

        if (png.getPixelType() != PixelType::Rgba8u) {
            EXCEPTION("Image must be of type RGBA 8-bit");
        }

        image = assetManager.addToAtlas(png.getSize(), png.getData());
    } catch (...) {
        EXCEPTION_NESTED("Failed to load image: '{}'", getName());
    }
}

std::shared_ptr<AssetImage> AssetImage::from(const std::string& name) {
    return AssetManager::singleton().find<AssetImage>(name);
}
