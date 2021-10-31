#pragma once

#include "../Config.hpp"
#include "../Modding/Manifest.hpp"
#include <memory>

namespace Scissio {
class AssetManager;

class SCISSIO_API Asset {
public:
    explicit Asset(const Manifest& mod, std::string name) : mod(mod), name(std::move(name)) {
    }
    virtual ~Asset() = default;

    virtual void load(AssetManager& assetManager) = 0;

    const Manifest& getMod() const {
        return mod;
    }

    const std::string& getName() const {
        return name;
    }

private:
    const Manifest& mod;
    std::string name;
};

using AssetPtr = std::shared_ptr<Asset>;
} // namespace Scissio
