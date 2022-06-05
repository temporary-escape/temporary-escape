#include "AssetFontFamily.hpp"

#include "../Utils/Exceptions.hpp"

using namespace Engine;

AssetFontFamily::AssetFontFamily(const Manifest& mod, std::string name) : Asset(mod, std::move(name)) {
}

void AssetFontFamily::load(AssetManager& assetManager, bool noGraphics) {
    (void)assetManager;
    (void)noGraphics;
}

void AssetFontFamily::add(std::string name, AssetFontFacePtr face) {
    faces.insert(std::make_pair(std::move(name), std::move(face)));
}

const AssetFontFacePtr& AssetFontFamily::get(const std::string& name) const {
    try {
        return faces.at(name);
    } catch (const std::out_of_range& e) {
        (void)e;
        EXCEPTION("Font face '{}' of family '{}' not found", name, getName());
    }
}
