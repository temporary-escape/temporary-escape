#pragma once

#include "../Graphics/Texture2D.hpp"
#include "Icon.hpp"

namespace Scissio {
class SCISSIO_API IconAtlas : public Asset {
public:
    explicit IconAtlas(const Manifest& mod, std::string name, Path path);
    virtual ~IconAtlas() = default;

    void load(AssetManager& assetManager) override;
    void add(std::string name, IconPtr icon);
    const IconPtr& get(const std::string& name) const;
    const Texture2D& getTexture() const {
        return texture;
    }

    const Vector2i& getSize() const {
        return size;
    }

private:
    Path path;
    std::unordered_map<std::string, IconPtr> icons;
    Vector2i size;
    Texture2D texture{NO_CREATE};
};

using IconAtlasPtr = std::shared_ptr<IconAtlas>;
} // namespace Scissio
