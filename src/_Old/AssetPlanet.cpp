#include "AssetPlanet.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetPlanet::AssetPlanet(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetPlanet::load(AssetManager& assetManager, bool noGraphics) {
    (void)noGraphics;

    try {
        definition.fromYaml(path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load planet: '{}'", getName());
    }
}

std::shared_ptr<AssetPlanet> AssetPlanet::from(const std::string& name) {
    return AssetManager::singleton().find<AssetPlanet>(name);
}
