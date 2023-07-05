#pragma once

#include "../../assets/primitive.hpp"
#include "../component.hpp"
#include "../grid.hpp"
#include "component_debug.hpp"

namespace Engine {
class ENGINE_API ComponentStarFlare : public Component {
public:
    ComponentStarFlare() = default;
    explicit ComponentStarFlare(entt::registry& reg, entt::entity handle, TexturePtr texture, TexturePtr textureLow,
                                TexturePtr textureHigh);
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

    const Vector2& getSize() const {
        return size;
    }

    void setSize(const Vector2& value) {
        size = value;
    }

    float getTemperature() const {
        return temperature;
    }

    void setTemperature(const float value) {
        temperature = value;
    }

    bool isBackground() const {
        return background;
    }

    void setBackground(const bool value) {
        background = value;
    }

private:
    TexturePtr texture;
    TexturePtr textureLow;
    TexturePtr textureHigh;
    Mesh mesh;
    Vector2 size{0.3f, 0.3f};
    float temperature{0.5f};
    bool background{true};
};
} // namespace Engine
