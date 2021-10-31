#include "ComponentParticleEmitter.hpp"

using namespace Scissio;

ComponentParticleEmitter::ComponentParticleEmitter() : Component(Type), texture{nullptr}, mesh{NO_CREATE} {
}

ComponentParticleEmitter::ComponentParticleEmitter(Object& object, AssetTexturePtr texture)
    : Component(Type, object), mesh{NO_CREATE}, texture{std::move(texture)} {
}

void ComponentParticleEmitter::render(Shader& shader) {
    if (!mesh) {
        mesh = Mesh{};
        mesh.setCount(128);
        mesh.setPrimitive(PrimitiveType::Points);
    }

    // shader.setModelMatrix(getObject().getTransform());
    // shader.bindParticleTexture(texture->getTexture());
    shader.draw(mesh);
}
