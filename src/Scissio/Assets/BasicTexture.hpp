#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"

namespace Scissio {
class SCISSIO_API BasicTexture : public Asset {
public:
    explicit BasicTexture(const Manifest& mod, std::string name, const Path& path);
    virtual ~BasicTexture() = default;

    void load(AssetManager& assetManager) override;

    const Texture2D& getTexture() const {
        return texture;
    }

private:
    Path path;
    Texture2D texture;
};

using BasicTexturePtr = std::shared_ptr<BasicTexture>;
} // namespace Scissio

MSGPACK_CONVERT(Scissio::BasicTexturePtr)
