#include "ComponentParticleEmitter.hpp"

using namespace Scissio;

ComponentParticleEmitter::ComponentParticleEmitter(Object& object, AssetTexturePtr texture)
    : Component(object), mesh{NO_CREATE}, texture{std::move(texture)} {
}
