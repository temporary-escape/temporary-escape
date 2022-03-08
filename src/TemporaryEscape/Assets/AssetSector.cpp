#include "AssetSector.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetSector::AssetSector(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetSector::load(AssetManager& assetManager) {
    (void)assetManager;
}

std::shared_ptr<AssetSector> AssetSector::from(const std::string& name) {
    return AssetManager::singleton().find<AssetSector>(name);
}
