#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Math/Vector.hpp"
#include "Asset.hpp"

namespace Scissio {
class ImageAtlas;

class SCISSIO_API Image : public Asset {
public:
    explicit Image(const Manifest& mod, const std::string& name, ImageAtlas& atlas, Path path);
    explicit Image(const Manifest& mod, const std::string& name, ImageAtlas& atlas, const Texture2D& texture,
                   const Vector2i& pos, const Vector2i& size);
    virtual ~Image() = default;

    void load(AssetManager& assetManager) override;

    const Texture2D& getTexture() const {
        assert(!!texture);
        return *texture;
    }

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
    ImageAtlas& atlas;
    const Texture2D* texture;
    Path path;
    Vector2i pos;
    Vector2i size;
    Canvas2D::Image image;
};

using ImagePtr = std::shared_ptr<Image>;
} // namespace Scissio
