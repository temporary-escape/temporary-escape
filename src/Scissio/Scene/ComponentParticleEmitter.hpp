#pragma once
#include "../Assets/AssetTexture.hpp"
#include "../Graphics/Mesh.hpp"
#include "Component.hpp"

namespace Scissio {
class ComponentParticleEmitter : public Component {
public:
    ComponentParticleEmitter() = default;
    explicit ComponentParticleEmitter(Object& object, AssetTexturePtr texture);
    virtual ~ComponentParticleEmitter() = default;

private:
    Mesh mesh{NO_CREATE};
    AssetTexturePtr texture{nullptr};

public:
    MSGPACK_DEFINE_ARRAY(texture);
};
} // namespace Scissio
