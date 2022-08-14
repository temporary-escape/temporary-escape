#pragma once

#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentSkybox : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentSkybox() = default;
    explicit ComponentSkybox(Object& object, const Color4& color) : Component{object}, color{color} {
    }
    explicit ComponentSkybox(Object& object, uint64_t seed) : Component{object}, seed{seed} {
    }
    virtual ~ComponentSkybox() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    uint64_t getSeed() const {
        return seed;
    }

    const Color4& getColor() const {
        return color;
    }

    /*const Skybox& getSkybox() const {
        return skybox;
    }*/

    /*void recalculate(SkyboxRenderer& skyboxRenderer) {
        if (!skybox.texture) {
            skybox = skyboxRenderer.renderAndFilter(seed);
        }
    }*/

private:
    uint64_t seed{0};
    Color4 color;
    VulkanTexture texture;

public:
    MSGPACK_DEFINE_ARRAY(seed, color);
};
} // namespace Engine
