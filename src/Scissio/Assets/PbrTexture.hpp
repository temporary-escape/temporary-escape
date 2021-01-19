#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Texture2D.hpp"
#include "Asset.hpp"

namespace Scissio {
class SCISSIO_API PbrTexture : public Asset {
public:
    explicit PbrTexture(const Manifest& mod, std::string name, const Path& path, PbrTextureType type);
    virtual ~PbrTexture() = default;

    void load(AssetManager& assetManager) override;

    const Texture2D& getTexture() const {
        return texture;
    }

    PbrTextureType getType() const {
        return type;
    }

private:
    Path path;
    PbrTextureType type;
    Texture2D texture;
};

using PbrTexturePtr = std::shared_ptr<PbrTexture>;
} // namespace Scissio
