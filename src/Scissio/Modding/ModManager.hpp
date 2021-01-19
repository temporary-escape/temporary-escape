#pragma once

#include "../Assets/AssetManager.hpp"
#include "Manifest.hpp"

namespace Scissio {
class SCISSIO_API ModManager {
public:
    explicit ModManager(AssetManager& assetManager);
    virtual ~ModManager() = default;

    void load(const Path& dir);

private:
    AssetManager& assetManager;
};
} // namespace Scissio
