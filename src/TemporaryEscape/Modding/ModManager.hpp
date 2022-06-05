#pragma once

#include "../Assets/AssetManager.hpp"
#include "Manifest.hpp"

namespace Engine {
class Database;

class ENGINE_API ModManager {
public:
    ModManager();
    virtual ~ModManager() = default;

    void load(AssetManager& assetManager, const Path& dir);

    const std::vector<std::shared_ptr<Manifest>>& getManifests() const {
        return manifests;
    }

private:
    std::vector<std::shared_ptr<Manifest>> manifests;
};
} // namespace Engine
