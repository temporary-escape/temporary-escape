#include "FontFamily.hpp"

#include "../Utils/Exceptions.hpp"

using namespace Scissio;

FontFamily::FontFamily(const Manifest& mod, std::string name) : Asset(mod, std::move(name)) {
}

void FontFamily::load(AssetManager& assetManager) {
    (void)assetManager;
    Log::d("Loading font family: '{}'", getName());
}

void FontFamily::add(std::string name, FontFacePtr face) {
    faces.insert(std::make_pair(std::move(name), std::move(face)));
}

const FontFacePtr& FontFamily::get(const std::string& name) const {
    try {
        return faces.at(name);
    } catch (const std::out_of_range& e) {
        (void)e;
        EXCEPTION("Font face '{}' of family '{}' not found", name, getName());
    }
}
