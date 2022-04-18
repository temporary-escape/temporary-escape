#pragma once

#include "../Assets/SkyboxRenderer.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentSkybox : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentSkybox() = default;
    explicit ComponentSkybox(Object& object, uint64_t seed) : Component(object), skybox{}, seed(seed) {
    }
    explicit ComponentSkybox(Object& object, Skybox skybox) : Component(object), skybox(std::move(skybox)), seed(0U) {
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

    const Skybox& getSkybox() const {
        return skybox;
    }

    void recalculate(SkyboxRenderer& skyboxRenderer) {
        if (!skybox.texture) {
            skybox = skyboxRenderer.renderAndFilter(seed);
        }
    }

private:
    Skybox skybox;
    uint64_t seed;

public:
    MSGPACK_DEFINE_ARRAY(seed);
};
} // namespace Engine
