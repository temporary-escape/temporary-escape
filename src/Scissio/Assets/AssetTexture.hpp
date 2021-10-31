#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"

namespace Scissio {
class SCISSIO_API AssetTexture : public Asset {
public:
    explicit AssetTexture(const Manifest& mod, std::string name, const Path& path, TextureType type);
    virtual ~AssetTexture() = default;

    void load(AssetManager& assetManager) override;

    const Texture2D& getTexture() const {
        return texture;
    }

    TextureType getType() const {
        return type;
    }

private:
    Path path;
    TextureType type;
    Texture2D texture;
};

using AssetTexturePtr = std::shared_ptr<AssetTexture>;

namespace Xml {
template <> void Node::convert<AssetTexturePtr>(AssetTexturePtr& value) const;
} // namespace Xml
} // namespace Scissio

MSGPACK_CONVERT(Scissio::AssetTexturePtr)
