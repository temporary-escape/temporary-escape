#include "AssetEntity.hpp"
#include "AssetManager.hpp"
#include <wrenbind17/wrenbind17.hpp>

using namespace Engine;

AssetEntity::AssetEntity(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetEntity::load(AssetManager& assetManager) {
    (void)assetManager;
    className = path.stem().string();
    const auto dir = getMod().path.stem().string();
    moduleName = fmt::format("{}/entities/{}", dir, className);
}

void AssetEntity::run(wrenbind17::VM& vm) {
    try {
        vm.runFromModule(moduleName);
        auto k = vm.find(moduleName, className); // Find the class
        if (!k) {
            EXCEPTION("Module has no class '{}'", className);
        }

    } catch (...) {
        EXCEPTION_NESTED("Failed to load entity script '{}'", moduleName);
    }
}

std::shared_ptr<AssetEntity> AssetEntity::from(const std::string& name) {
    return AssetManager::singleton().find<AssetEntity>(name);
}
