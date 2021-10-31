#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Math/Vector.hpp"
#include "Asset.hpp"

namespace Scissio {
class IconAtlas;

class SCISSIO_API Icon : public Asset {
public:
    explicit Icon(const Manifest& mod, const std::string& name, const IconAtlas& atlas, const Vector2i& pos,
                  const Vector2i& size);
    virtual ~Icon() = default;

    void load(AssetManager& assetManager) override;

    const Texture2D& getTexture() const;

    const Vector2i& getPos() const {
        return pos;
    }

    const Vector2i& getSize() const {
        return size;
    }

    const Canvas2D::Image& getImage() const {
        return image;
    }

    const Vector2i& getAtlasSize() const;

private:
    const IconAtlas& atlas;
    Vector2i pos;
    Vector2i size;
    Canvas2D::Image image;
};

using IconPtr = std::shared_ptr<Icon>;
} // namespace Scissio
