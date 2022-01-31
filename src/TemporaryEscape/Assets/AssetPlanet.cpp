#include "AssetPlanet.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetPlanet::AssetPlanet(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetPlanet::load(AssetManager& assetManager) {
    try {
        Xml::Document(path).getRoot().convert(definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load planet: '{}'", getName());
    }
}

std::shared_ptr<AssetPlanet> AssetPlanet::from(const std::string& name) {
    return AssetManager::singleton().find<AssetPlanet>(name);
}

void Xml::Adaptor<AssetPlanet::Definition::Surface>::convert(const Xml::Node& n, AssetPlanet::Definition::Surface& v) {
    n.child("texture").convert(v.texture);
    n.child("offset").convert(v.offset);
}

void Xml::Adaptor<AssetPlanet::Definition>::convert(const Xml::Node& n, AssetPlanet::Definition& v) {
    n.child("surface").convert(v.surface);
    n.child("description").convert(v.description);
}
