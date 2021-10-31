#include "AssetFontFace.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

AssetFontFace::AssetFontFace(const Manifest& mod, const Config& config, const Path& path)
    : Asset(mod, path.stem().string()), config(config), path(path) {
}

void AssetFontFace::load(AssetManager& assetManager) {
    try {
        // font = renderer.createFontFace(path, config.guiFontSize);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load font face: '{}'", getName());
    }
}
