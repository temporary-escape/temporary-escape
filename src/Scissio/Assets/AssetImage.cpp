#include "AssetImage.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

AssetImage::AssetImage(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetImage::load(AssetManager& assetManager) {
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

void Xml::Adaptor<AssetImagePtr>::convert(const Xml::Node& n, AssetImagePtr& v) {
    v = AssetImage::from(n.asString());
}
