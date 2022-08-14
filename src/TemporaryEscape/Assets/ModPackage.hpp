#pragma once

#include "ModManifest.hpp"

namespace Engine {
class ENGINE_API ModPackage {
public:
    explicit ModPackage(const ModManifest& manifest) : manifest{manifest} {
    }

    const ModManifest& getManifest() const {
        return manifest;
    }

private:
    ModManifest manifest;
};
} // namespace Engine
