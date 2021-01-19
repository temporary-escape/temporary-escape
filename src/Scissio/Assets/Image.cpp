#include "Image.hpp"

#include "../Graphics/Canvas2D.hpp"
#include "../Utils/Log.hpp"
#include "AssetManager.hpp"
#include "IconAtlas.hpp"

using namespace Scissio;

Image::Image(const Manifest& mod, const std::string& name, const ImageAtlas& atlas, const Texture2D& texture,
             const Vector2i& pos, const Vector2i& size)
    : Asset(mod, name), atlas(atlas), texture(texture), pos(pos), size(size) {
}

void Image::load(AssetManager& assetManager) {
    (void)assetManager;
    // Log::d("Loading icon: '{}'", getName());
    try {
        image = assetManager.getCanvas().loadImage(*this);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load icon: '{}'", getName());
    }
}

const Vector2i& Image::getAtlasSize() const {
    return atlas.getSize();
}
