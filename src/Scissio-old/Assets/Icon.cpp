#include "Icon.hpp"

#include "../Graphics/Canvas2D.hpp"
#include "../Utils/Log.hpp"
#include "AssetManager.hpp"
#include "IconAtlas.hpp"

using namespace Scissio;

Icon::Icon(const Manifest& mod, const std::string& name, const IconAtlas& atlas, const Vector2i& pos,
           const Vector2i& size)
    : Asset(mod, name), atlas(atlas), pos(pos), size(size) {
}

void Icon::load(AssetManager& assetManager) {
    (void)assetManager;
    // Log::d("Loading icon: '{}'", getName());
    try {
        pos = Vector2{pos.x, atlas.getSize().y - pos.y - size.y};
        image = assetManager.getCanvas().loadImage(*this);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load icon: '{}'", getName());
    }
}

const Texture2D& Icon::getTexture() const {
    return atlas.getTexture();
}

const Vector2i& Icon::getAtlasSize() const {
    return atlas.getSize();
}
