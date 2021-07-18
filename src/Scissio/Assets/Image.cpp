#include "Image.hpp"

#include "../Graphics/Canvas2D.hpp"
#include "../Utils/Log.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"
#include "ImageAtlas.hpp"

using namespace Scissio;

Image::Image(const Manifest& mod, const std::string& name, ImageAtlas& atlas, Path path)
    : Asset(mod, name), atlas{atlas}, texture{nullptr}, path{std::move(path)} {
}

Image::Image(const Manifest& mod, const std::string& name, ImageAtlas& atlas, const Texture2D& texture,
             const Vector2i& pos, const Vector2i& size)
    : Asset(mod, name), atlas{atlas}, texture{&texture}, pos{pos}, size{size} {
}

void Image::load(AssetManager& assetManager) {
    (void)assetManager;

    Log::d("Loading image: '{}'", getName());

    try {
        if (!texture) {
            PngImporter png(path);

            const auto node = atlas.reserve(png.getSize());

            if (!node) {
                EXCEPTION("Failed to allocate texture atlas node of size: {}x{}", size.x, size.y);
            }

            texture = node->texture;

            node->texture->setPixels(0, {0, 0}, png.getSize(), png.getPixelType(), png.getData());

            pos = {0.0f, 0.0f};
            size = atlas.getSize();
        }

        image = assetManager.getCanvas().loadImage(*this);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load image: '{}'", getName());
    }
}

const Vector2i& Image::getAtlasSize() const {
    return atlas.getSize();
}
