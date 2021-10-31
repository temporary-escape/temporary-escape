#pragma once
#include "../Assets/AssetTexture.hpp"
#include "../Graphics/Mesh.hpp"
#include "Component.hpp"

namespace Scissio {
class ComponentParticleEmitter : public Component {
public:
    static constexpr ComponentType Type = 7;

    ComponentParticleEmitter();
    explicit ComponentParticleEmitter(Object& object, AssetTexturePtr texture);
    virtual ~ComponentParticleEmitter() = default;

    void render(Shader& shader);

private:
    Mesh mesh;
    AssetTexturePtr texture;

public:
    MSGPACK_DEFINE_ARRAY(texture);
};
} // namespace Scissio
