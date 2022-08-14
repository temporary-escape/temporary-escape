#include "AssetTurret.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetTurret::AssetTurret(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetTurret::load(AssetManager& assetManager, bool noGraphics) {
    (void)noGraphics;

    try {
        definition.fromYaml(path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load turret: '{}'", getName());
    }
}

std::shared_ptr<AssetTurret> AssetTurret::from(const std::string& name) {
    return AssetManager::singleton().find<AssetTurret>(name);
}
