#pragma once

#include "../Assets/AssetManager.hpp"
#include "Manifest.hpp"

namespace Scissio {
class Database;

class SCISSIO_API ModManager {
public:
    explicit ModManager(AssetManager& assetManager);
    virtual ~ModManager() = default;

    void load(const Path& dir);
    void loadXmlData(const Path& dir, Database& db);

private:
    AssetManager& assetManager;
};
} // namespace Scissio
