#include "FontFace.hpp"
#include "../Graphics/Canvas2D.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

FontFace::FontFace(const Manifest& mod, const Path& path) : Asset(mod, path.stem().string()), path(path), handle(-1) {
}

void FontFace::load(AssetManager& assetManager) {
    Log::d("Loading font face: '{}'", getName());
    try {
        handle = assetManager.getCanvas().loadFont(path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load font face: '{}'", getName());
    }
}
