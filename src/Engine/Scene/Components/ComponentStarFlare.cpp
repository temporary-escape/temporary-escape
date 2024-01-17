#include "ComponentStarFlare.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentStarFlare::ComponentStarFlare(EntityId entity, TexturePtr texture, TexturePtr textureLow,
                                       TexturePtr textureHigh) :
    Component{entity},
    texture{std::move(texture)},
    textureLow{std::move(textureLow)},
    textureHigh{std::move(textureHigh)} {

    mesh.count = 4;
    mesh.instances = 1;
}
