#include "AssetParticles.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetParticles::AssetParticles(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetParticles::load(AssetManager& assetManager) {
    try {
        definition.fromYaml(path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load particles: '{}'", getName());
    }
}

std::shared_ptr<AssetParticles> AssetParticles::from(const std::string& name) {
    return AssetManager::singleton().find<AssetParticles>(name);
}
