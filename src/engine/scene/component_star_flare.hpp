#pragma once

#include "../assets/primitive.hpp"
#include "component.hpp"
#include "component_debug.hpp"
#include "grid.hpp"

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API ComponentStarFlare : public Component {
public:
    ComponentStarFlare() = default;
    explicit ComponentStarFlare(TexturePtr texture, TexturePtr textureLow, TexturePtr textureHigh);
    virtual ~ComponentStarFlare() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentStarFlare);

    const TexturePtr& getTexture() const {
        return texture;
    }

    const TexturePtr& getTextureLow() const {
        return textureLow;
    }

    const TexturePtr& getTextureHigh() const {
        return textureHigh;
    }

    const Mesh& getMesh() const {
        return mesh;
    }

private:
    TexturePtr texture;
    TexturePtr textureLow;
    TexturePtr textureHigh;
    Mesh mesh;
};
} // namespace Engine
