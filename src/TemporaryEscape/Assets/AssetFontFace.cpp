#include "AssetFontFace.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetFontFace::AssetFontFace(const Manifest& mod, const Config& config, const Path& path)
    : Asset(mod, path.stem().string()), config(config), path(path), font(0) {
}

void AssetFontFace::load(AssetManager& assetManager) {
    try {
        font = assetManager.createFontHandle(path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load font face: '{}'", getName());
    }
}
