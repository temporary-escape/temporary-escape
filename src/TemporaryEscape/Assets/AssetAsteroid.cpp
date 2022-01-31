#include "AssetAsteroid.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetAsteroid::AssetAsteroid(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetAsteroid::load(AssetManager& assetManager) {
    try {
        Xml::Document(path).getRoot().convert(definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load asteroid: '{}'", getName());
    }
}

std::shared_ptr<AssetAsteroid> AssetAsteroid::from(const std::string& name) {
    return AssetManager::singleton().find<AssetAsteroid>(name);
}

void Xml::Adaptor<AssetAsteroid::Definition>::convert(const Xml::Node& n, AssetAsteroid::Definition& v) {
    n.child("name").convert(v.name);
    n.child("models").convert(v.models);
}
