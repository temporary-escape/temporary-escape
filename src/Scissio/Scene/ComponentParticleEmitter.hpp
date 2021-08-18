#pragma once
#include "../Assets/BasicTexture.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Shaders/ShaderParticleEmitter.hpp"
#include "Component.hpp"

namespace Scissio {
class ComponentParticleEmitter : public Component {
public:
    static constexpr ComponentType Type = 7;

    ComponentParticleEmitter();
    explicit ComponentParticleEmitter(Object& object, BasicTexturePtr texture);
    virtual ~ComponentParticleEmitter() = default;

    void render(ShaderParticleEmitter& shader);

private:
    Mesh mesh;
    BasicTexturePtr texture;

public:
    MSGPACK_DEFINE_ARRAY(texture);
};
} // namespace Scissio
