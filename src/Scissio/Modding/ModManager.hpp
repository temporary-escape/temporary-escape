#pragma once

#include "../Assets/AssetManager.hpp"
#include "Manifest.hpp"

namespace Scissio {
class Database;

class SCISSIO_API ModManager {
public:
    explicit ModManager();
    virtual ~ModManager() = default;

    void load(AssetManager& assetManager, const Path& dir);
    void loadXmlData(AssetManager& assetManager, const Path& dir, Database& db);

private:
    std::vector<std::shared_ptr<Manifest>> manifests;
};
} // namespace Scissio
